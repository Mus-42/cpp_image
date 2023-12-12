#ifndef _CPP_IMAGE_BMP_COMMON_HPP_
#define _CPP_IMAGE_BMP_COMMON_HPP_

namespace img::bmp {
    namespace impl {
        constexpr size_t bmp_file_header_size = 14;

        constexpr size_t dib_v1_header_size = 40;
        // NOTE: can't read v2 v3 because it's undocumented
        constexpr size_t dib_v4_header_size = 108;
        constexpr size_t dib_v5_header_size = 124;

        constexpr uint32_t sRGB_magic = 0x73524742;

        struct DibV5HeaderData {
            uint32_t width;
            uint32_t height;

            uint16_t planes = 1; 
            uint16_t bpp;

            uint32_t compression = 0;
            uint32_t image_size;

            uint32_t x_ppm;
            uint32_t y_ppm;

            uint32_t color_table_size = 0;
            uint32_t important_colors = 0;

            uint32_t r_mask = 0;
            uint32_t g_mask = 0;
            uint32_t b_mask = 0;
            uint32_t a_mask = 0;

            uint32_t color_space_type = 0;//sRGB_magic;
            uint32_t color_space_endpoints[9]{};

            uint32_t gamma_r = 0;
            uint32_t gamma_g = 0;
            uint32_t gamma_b = 0;

            uint32_t intent = 0;//0x00000008; // Absolute Colorimetric

            uint32_t icc_profile_data = 0;
            uint32_t icc_profile_size = 0;

            uint32_t _reserved = 0;
        };

        static_assert(sizeof(DibV5HeaderData) + 4 == dib_v5_header_size);
    } // namespace impl
} // img::bmp

#endif//_CPP_IMAGE_BMP_COMMON_HPP_