#include <cpp_image/bmp.hpp>
#include <fstream>
#include <concepts>
#include <array>

#include "bmp_common.hpp"

namespace img::bmp {
    namespace impl {
        static inline void write_le_int(std::ofstream& out, std::integral auto val) {
            static constexpr size_t N = sizeof(val);
            std::array<char, N> val_bytes;
            for (size_t i = 0; i < N; i++)
                val_bytes[i] = static_cast<char>(static_cast<unsigned char>(val >> (i * 8) & 0xFF));
            out.write(val_bytes.data(), N);
        }

        static inline uint32_t img_data_size(uint32_t w, uint32_t h, uint16_t bpp) {
            uint32_t row_size = (bpp * w + 31) / 32 * 4;
            return row_size * h;
        }


        static inline void write_bmp_file_header(std::ofstream& out, uint32_t file_size, uint32_t image_data_offset) {
            static constexpr std::array<char, 2> magic = { 'B', 'M' };
            out.write(magic.data(), 2);
            write_le_int(out, file_size);
            static constexpr std::array<char, 4> padding = { 0, 0, 0, 0 };
            out.write(padding.data(), 4);
            write_le_int(out, image_data_offset);
        }
        
        static inline DibV5HeaderData build_dib_v5_header(const Image& img, const BMPWriteOptions& options) {
            const auto [width, height] = img.dimensions();
            uint16_t bpp = static_cast<uint16_t>(options.bpp);
            uint32_t img_size = img_data_size(width, height, bpp);
            auto data = DibV5HeaderData {
                .width = width,
                .height = height,
                .bpp = bpp,
                .image_size = img_size,
                .x_ppm = options.x_ppm,
                .y_ppm = options.y_ppm,
            };
            if (options.bpp >= BitsPerPixel::Sixteen) {
                data.compression = 3; // BI_BITFIELDS
                if (options.bpp == BitsPerPixel::Sixteen) {
                    data.r_mask = 0x00F0;
                    data.g_mask = 0x000F;
                    data.b_mask = 0xF000;
                    data.a_mask = 0x0F00;
                } else {
                    data.r_mask = 0x000000FF;
                    data.g_mask = 0x0000FF00;
                    data.b_mask = 0x00FF0000;
                    data.a_mask = 0xFF000000;
                }
            }
            return data;
        }


        static inline void write_dib_v5_header(std::ofstream& out, const DibV5HeaderData& data) {
            write_le_int(out, static_cast<uint32_t>(dib_v5_header_size));
            write_le_int(out, data.width);
            write_le_int(out, data.height);

            write_le_int(out, data.planes);
            write_le_int(out, data.bpp);

            write_le_int(out, data.compression);
            write_le_int(out, data.image_size);
            
            write_le_int(out, data.x_ppm);
            write_le_int(out, data.y_ppm);

            write_le_int(out, data.color_table_size);
            write_le_int(out, data.important_colors);

            write_le_int(out, data.r_mask);
            write_le_int(out, data.g_mask);
            write_le_int(out, data.b_mask);
            write_le_int(out, data.a_mask);

            write_le_int(out, data.color_space_type);
            for (size_t i = 0; i < 9; i++)
                write_le_int(out, data.color_space_endpoints[i]);
            
            write_le_int(out, data.gamma_r);
            write_le_int(out, data.gamma_g);
            write_le_int(out, data.gamma_b);

            write_le_int(out, data.intent);

            write_le_int(out, data.icc_profile_data);
            write_le_int(out, data.icc_profile_size);

            write_le_int(out, data._reserved);
        }

        static inline void write_color_table(std::ofstream& out, BitsPerPixel bpp) {
            switch (bpp) {
            case BitsPerPixel::One: {
                static constexpr char color_table[8] = {
                    '\0', '\0', '\0', '\xFF',
                    '\xFF', '\xFF', '\xFF', '\xFF'
                };
                out.write(color_table, 8);
            } break;
            case BitsPerPixel::Four: {
                static char color_table[4 * 16];
                for (size_t i = 0; i < 16; i++) {
                    color_table[i * 4 + 2] = (i >> 2 & 3) * 255 / 3;
                    color_table[i * 4 + 1] = (i >> 1 & 1) * 255;
                    color_table[i * 4 + 0] = (i & 1) * 255;
                }
                out.write(color_table, 4 * 16);
            } break;
            case BitsPerPixel::Eight: {
                static char color_table[4 * 256];
                for (size_t i = 0; i < 256; i++) {
                    color_table[i * 4 + 2] = (i >> 5 & 7) * 255 / 7;
                    color_table[i * 4 + 1] = (i >> 2 & 7) * 255 / 7;
                    color_table[i * 4 + 0] = (i & 3) * 255 / 3;
                }
                out.write(color_table, 4 * 256);
            } break;
            default: 
                break;
            }
        }

        static inline void write_color_to_buf(char* buf, uint32_t index, Color col, BitsPerPixel bpp) {
            switch (bpp) {
            case BitsPerPixel::One: {
                constexpr uint32_t white_threshold = 127 * 3 * 255;
                uint32_t pixel_val = ((uint32_t(col.r) >> 8) + (uint32_t(col.g) >> 8) + (uint32_t(col.b) >> 8)) * (uint32_t(col.a) >> 8);

                if (pixel_val > white_threshold) {
                    buf[index >> 3] |= 1 << (index & 0x7 ^ 0x7);
                }
            } break;
            case BitsPerPixel::Four: {
                buf[index >> 1] |= (col.r >> 14 << 2 | col.g >> 15 << 1 | col.b >> 15) << ((index & 1) * 4);
            } break;
            case BitsPerPixel::Eight: {
                buf[index] = col.r >> 13 << 5 | col.g >> 13 << 2 | col.b >> 14;
            } break;
            case BitsPerPixel::Sixteen: {
                buf[index * 2 + 0] = col.r >> 12 << 4 | col.g >> 12;
                buf[index * 2 + 1] = col.b >> 12 << 4 | col.a >> 12;
            } break;
            case BitsPerPixel::ThirtyTwo: {
                // write color as RGBA32
                buf[index * 4 + 0] = col.r >> 8;
                buf[index * 4 + 1] = col.g >> 8;
                buf[index * 4 + 2] = col.b >> 8;
                buf[index * 4 + 3] = col.a >> 8;
            } break;
            default:
                break;
            }
        }

        static inline void write_pixel_data(std::ofstream& out, const Image& img, const BMPWriteOptions& options, uint32_t padd_size) {
            static constexpr char padd[4] = {0, 0, 0, 0};
            out.write(padd, padd_size); 
            const auto [width, height] = img.dimensions();
            uint16_t bpp = static_cast<uint16_t>(options.bpp);
            uint32_t row_size = (bpp * width + 31) / 32 * 4;
            std::vector<char> buf(row_size);

            for (uint32_t i = height-1; i < height; i--) {
                memset(buf.data(), 0, row_size);
                for (uint32_t j = 0; j < width; j++) {
                    Color col = img[{i, j}];
                    write_color_to_buf(buf.data(), j, col, options.bpp);
                }
                out.write(buf.data(), row_size);
            }
        }
    } // namespace impl

    void write_image(const std::filesystem::path& path, const Image& img, const BMPWriteOptions& options) {
        const auto dib_header = impl::build_dib_v5_header(img, options);
        
        uint32_t img_size = dib_header.image_size;
        uint32_t img_data_offset = impl::bmp_file_header_size + impl::dib_v5_header_size;
        if (options.bpp <= BitsPerPixel::Eight) {
            img_data_offset += 4 << dib_header.bpp;
        }
        uint32_t img_data_offset_aligned = (img_data_offset + 3) / 4 * 4;
        uint32_t padd_size = img_data_offset_aligned - img_data_offset;
        uint32_t file_size = img_data_offset_aligned + img_size; 

        std::ofstream out(path, std::ios::binary);
        
        impl::write_bmp_file_header(out, file_size, img_data_offset_aligned);
        impl::write_dib_v5_header(out, dib_header);
        if (options.bpp <= BitsPerPixel::Eight) {
            impl::write_color_table(out, options.bpp);
        }
        impl::write_pixel_data(out, img, options, padd_size);

    }
} // namespace img::bmp