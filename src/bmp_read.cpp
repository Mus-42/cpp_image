#include <cpp_image/bmp.hpp>
#include <cpp_image/exceptions.hpp>

#include <fstream>
#include <concepts>
#include <array>
#include <bit>

#include <iostream>

#include "bmp_common.hpp"


namespace img::bmp {
    namespace impl {
        static inline void read_le_int(std::ifstream& in, std::integral auto& val) {
            using T = std::remove_cvref_t<decltype(val)>;
            static constexpr size_t N = sizeof(T);
            std::array<char, N> val_bytes;
            in.read(val_bytes.data(), N);
            val = 0;
            for (size_t i = 0; i < N; i++)
                val |= static_cast<T>(static_cast<unsigned char>(val_bytes[i])) << (i * 8);
        }

        static inline void read_bmp_file_header(std::ifstream& in, uint32_t& file_size, uint32_t& image_data_offset) {
            static constexpr std::array<char, 2> magic = { 'B', 'M' };
            std::array<char, 2> magic_buf;
            
            in.read(magic_buf.data(), 2);

            if (magic != magic_buf) {
                throw BmpReadError(BmpReadErrorKind::INVALID_MAGIC);
            }

            read_le_int(in, file_size);

            std::array<char, 4> padd;
            in.read(padd.data(), 4);

            read_le_int(in, image_data_offset);

            if (in.eof()) {
                throw BmpReadError(BmpReadErrorKind::UNEXPECTED_EOF);
            }
        }

        // NOTE: in case of version older than V5 fields rest of fields are not changed
        static inline void read_dib_header(std::ifstream& in, DibV5HeaderData& data) {
            uint32_t header_size = 0;
            read_le_int(in, header_size);

            if (
                header_size != static_cast<uint32_t>(dib_v1_header_size) &&
                header_size != static_cast<uint32_t>(dib_v4_header_size) &&
                header_size != static_cast<uint32_t>(dib_v5_header_size)
            ) {
                throw BmpReadError(BmpReadErrorKind::UNSUPPORTED_BITMAP_HEADER);
            }

            read_le_int(in, data.width);
            read_le_int(in, data.height);

            read_le_int(in, data.planes);
            read_le_int(in, data.bpp);

            read_le_int(in, data.compression);
            read_le_int(in, data.image_size);
            
            read_le_int(in, data.x_ppm);
            read_le_int(in, data.y_ppm);

            read_le_int(in, data.color_table_size);
            read_le_int(in, data.important_colors);

            if (header_size == static_cast<uint32_t>(dib_v1_header_size)) return;

            read_le_int(in, data.r_mask);
            read_le_int(in, data.g_mask);
            read_le_int(in, data.b_mask);
            read_le_int(in, data.a_mask);

            read_le_int(in, data.color_space_type);
            for (size_t i = 0; i < 9; i++)
                read_le_int(in, data.color_space_endpoints[i]);
            
            read_le_int(in, data.gamma_r);
            read_le_int(in, data.gamma_g);
            read_le_int(in, data.gamma_b);

            if (header_size == static_cast<uint32_t>(dib_v4_header_size)) return;

            read_le_int(in, data.intent);

            read_le_int(in, data.icc_profile_data);
            read_le_int(in, data.icc_profile_size);

            read_le_int(in, data._reserved);
        }

        struct ColorTableEntry {
            uint8_t r, g, b;
        };

        static inline std::vector<ColorTableEntry> read_color_table(std::ifstream& in, size_t entry_count) {
            std::array<uint8_t, 4> rgb_triplet;

            std::vector<ColorTableEntry> color_table(entry_count);
            for (size_t i = 0; i < entry_count; i++) {
                in.read(reinterpret_cast<char*>(rgb_triplet.data()), 4);
                color_table[i] = ColorTableEntry {
                    static_cast<uint8_t>(rgb_triplet[2]),
                    static_cast<uint8_t>(rgb_triplet[1]),
                    static_cast<uint8_t>(rgb_triplet[0]),
                };
            }

            return color_table;
        }

        static inline size_t get_color_index(const unsigned char* data, size_t i, uint32_t bpp) noexcept {
            switch (bpp) {
            case 1:
                return data[i >> 3] >> (i & 7 ^ 7) & 1;
            case 4:
                return data[i >> 1] >> ((i & 1 ^ 1) << 2) & 15;
            case 8:
                return data[i];
            // indexed color bpp can't be greater than 8
            default:
                return 0;
            }
        }

        static inline Image read_pixel_data_indexded(std::ifstream& in, const std::vector<ColorTableEntry>& table, uint32_t pixel_data_offset, const DibV5HeaderData& header) {
            in.seekg(static_cast<std::streampos>(pixel_data_offset));

            Image img = Image::Image(header.width, header.height);

            uint32_t row_size = (header.bpp * header.width + 31) / 32 * 4;
            std::vector<char> buf(row_size);

            for (uint32_t i = header.height-1; i < header.height; i--) {
                if (in.eof()) {
                    throw BmpReadError(BmpReadErrorKind::UNEXPECTED_EOF);
                }

                in.read(buf.data(), row_size);
                for (uint32_t j = 0; j < header.width; j++) {
                    size_t index = get_color_index(reinterpret_cast<const unsigned char*>(buf.data()), j, header.bpp);
                    auto col = table[index];
                    
                    img[{i, j}] = Color(col.r << 8, col.g << 8, col.b << 8);
                }
            }

            return img;
        }

        static inline uint32_t get_color_u32(const unsigned char* data, size_t i, uint32_t bpp) noexcept {
            switch (bpp) {
            case 16:
                return data[i * 2 + 0] | data[i * 2 + 1] << 8;
            case 24:
                return data[i * 3 + 0] | data[i * 3 + 1] << 8 | data[i * 3 + 2] << 16;
            case 32:
                return data[i * 4 + 0] | data[i * 4 + 1] << 8 | data[i * 4 + 2] << 16 | data[i * 4 + 3] << 24;
            default:
                return 0;
            }
        }

        static inline Image read_pixel_data_bitfield(std::ifstream& in, uint32_t pixel_data_offset, const DibV5HeaderData& header) {
            in.seekg(static_cast<std::streampos>(pixel_data_offset));

            Image img = Image::Image(header.width, header.height);

            uint32_t row_size = (header.bpp * header.width + 31) / 32 * 4;
            std::vector<char> buf(row_size);


            uint32_t r_popcnt = std::popcount(header.r_mask);
            uint32_t g_popcnt = std::popcount(header.g_mask);
            uint32_t b_popcnt = std::popcount(header.b_mask);
            uint32_t a_popcnt = std::popcount(header.a_mask);

            uint32_t r_shr = std::countr_zero(header.r_mask) + std::max(r_popcnt, UINT32_C(16)) - 16;
            uint32_t g_shr = std::countr_zero(header.g_mask) + std::max(g_popcnt, UINT32_C(16)) - 16;
            uint32_t b_shr = std::countr_zero(header.b_mask) + std::max(b_popcnt, UINT32_C(16)) - 16;
            uint32_t a_shr = std::countr_zero(header.a_mask) + std::max(a_popcnt, UINT32_C(16)) - 16;

            uint32_t r_shl = 16 - std::min(r_popcnt, UINT32_C(16));
            uint32_t g_shl = 16 - std::min(g_popcnt, UINT32_C(16));
            uint32_t b_shl = 16 - std::min(b_popcnt, UINT32_C(16));
            uint32_t a_shl = 16 - std::min(a_popcnt, UINT32_C(16));

            for (uint32_t i = header.height-1; i < header.height; i--) {
                if (in.eof()) {
                    throw BmpReadError(BmpReadErrorKind::UNEXPECTED_EOF);
                }

                in.read(buf.data(), row_size);
                for (uint32_t j = 0; j < header.width; j++) {
                    uint32_t col = get_color_u32(reinterpret_cast<const unsigned char*>(buf.data()), j, header.bpp);

                    uint16_t r = static_cast<uint16_t>(col >> r_shr << r_shl);
                    uint16_t g = static_cast<uint16_t>(col >> g_shr << g_shl);
                    uint16_t b = static_cast<uint16_t>(col >> b_shr << b_shl);
                    uint16_t a = static_cast<uint16_t>(col >> a_shr << a_shl);

                    img[{i, j}] = Color(r, g, b, a);
                }
            }

            return img;
        }

        static inline bool is_color_mask_valid(uint32_t mask) noexcept {
            auto diff = std::popcount(mask >> 1 ^ mask);
            if (diff > 2) return false;
            if (diff == 2 && (mask & 1) != 0) return false;
            return true;
        }
    } // namespace impl

    Image read_image(const std::filesystem::path& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open() || in.bad()) {
            throw BmpReadError(BmpReadErrorKind::CANT_OPEN_FILE);
        }

        uint32_t file_size = 0, image_data_offset = 0;

        impl::read_bmp_file_header(in, file_size, image_data_offset);
        
        impl::DibV5HeaderData header{};

        impl::read_dib_header(in, header);

        if (in.eof()) {
            throw BmpReadError(BmpReadErrorKind::UNEXPECTED_EOF);
        }

        // partially validate header
        if (
            header.bpp != 1 && header.bpp != 4 && header.bpp != 8 && header.bpp != 16 && header.bpp != 24 && header.bpp != 32 ||
            header.compression != 0 && header.compression != 3
        ) {
            throw BmpReadError(BmpReadErrorKind::INVALID_HEADER_DATA);
        }

        // indexded image
        if (header.bpp <= 8) {
            size_t table_size = header.color_table_size;
            if (table_size == 0) {
                table_size = 1ull << header.bpp;
            }
            const auto color_table = impl::read_color_table(in, table_size);
            return impl::read_pixel_data_indexded(in, color_table, image_data_offset, header);        
        }

        if (header.compression != 3) {
            if (header.bpp == 16) {
                header.r_mask = 0xF00;
                header.g_mask = 0x0F0;
                header.b_mask = 0x00F;
                header.a_mask = 0; // no alpha (((
            } else {
                header.r_mask = 0xFF0000;
                header.g_mask = 0x00FF00;
                header.b_mask = 0x0000FF;
                header.a_mask = 0; // no alpha (((
            }
        } else {
            // basic masks validation
            if (
                !impl::is_color_mask_valid(header.r_mask) ||
                !impl::is_color_mask_valid(header.g_mask) ||
                !impl::is_color_mask_valid(header.b_mask) ||
                !impl::is_color_mask_valid(header.a_mask)
            ) {
                throw BmpReadError(BmpReadErrorKind::INVALID_HEADER_DATA);
            }
        }

        return impl::read_pixel_data_bitfield(in, image_data_offset, header);
    }

    BMPImageInfo read_image_info(const std::filesystem::path& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open() || in.bad()) {
            throw BmpReadError(BmpReadErrorKind::CANT_OPEN_FILE);
        }

        uint32_t file_size = 0, image_data_offset = 0;

        impl::read_bmp_file_header(in, file_size, image_data_offset);
        
        impl::DibV5HeaderData header{};

        impl::read_dib_header(in, header);

        if (in.eof()) {
            throw BmpReadError(BmpReadErrorKind::UNEXPECTED_EOF);
        }

        // partially validate header
        if (
            header.bpp != 1 && header.bpp != 4 && header.bpp != 8 && header.bpp != 16 && header.bpp != 24 && header.bpp != 32 ||
            header.compression != 0 && header.compression != 3
        ) {
            throw BmpReadError(BmpReadErrorKind::INVALID_HEADER_DATA);
        }

        BMPImageInfo info{};

        info.bpp = static_cast<BitsPerPixel>(header.bpp);
        info.width = header.width;
        info.height = header.height;
        info.x_ppm = header.x_ppm;
        info.y_ppm = header.y_ppm;

        info.file_size = file_size;

        return info;
    }
} // namespace img::bmp