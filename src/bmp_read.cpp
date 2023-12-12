#include <cpp_image/bmp.hpp>
#include <cpp_image/exceptions.hpp>
#include <fstream>
#include <concepts>
#include <array>

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
                header_size != static_cast<uint32_t>(dib_v3_header_size) &&
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

            if (header_size == static_cast<uint32_t>(dib_v3_header_size)) return;

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

        static inline std::vector<ColorTableEntry> read_color_table(std::ifstream& in, BitsPerPixel bpp);

        static inline Image read_pixel_data()
    } // namespace impl

    Image read_image(const std::filesystem::path& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open() || in.bad()) {
            throw BmpReadError(BmpReadErrorKind::CANT_OPEN_FILE);
        }

        uint32_t file_size = 0, image_data_offset = 0;

        impl::read_bmp_file_header(in, file_size, image_data_offset);
        
        std::cout << "fs: " << file_size << " img_data: " << image_data_offset << std::endl;

        impl::DibV5HeaderData header{};

        impl::read_dib_header(in, header);

        if (in.eof()) {
            throw BmpReadError(BmpReadErrorKind::UNEXPECTED_EOF);
        }

        // partially validate header
        if (
            header.bpp != 1 && header.bpp != 4 && header.bpp != 8 && header.bpp != 16 && header.bpp != 32 ||
            header.compression != 0 && header.compression != 3
        ) {
            throw BmpReadError(BmpReadErrorKind::INVALID_HEADER_DATA);
        }

        if (header.compression != 3)

        return {};
    }
} // namespace img::bmp