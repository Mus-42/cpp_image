#ifndef _CPP_IMAGE_BMP_HPP_
#define _CPP_IMAGE_BMP_HPP_

#include <cstdint>
#include <filesystem>

#include "image.hpp"

namespace img::bmp {
    enum class BitsPerPixel: uint8_t {
        One = 1,
        Four = 4,
        Eight = 8,
        Sixteen = 16,
        ThirtyTwo = 32,
    };
    
    struct BMPWriteOptions {
        BitsPerPixel bpp = BitsPerPixel::ThirtyTwo;

        uint32_t x_ppm = 2400;
        uint32_t y_ppm = 2400;
        // TODO
    };

    struct BMPImageInfo {
        BitsPerPixel bpp;

        uint32_t width;
        uint32_t height;

        uint32_t x_ppm;
        uint32_t y_ppm;

        uint32_t file_size;
    };

    void write_image(const std::filesystem::path& path, const Image& img, const BMPWriteOptions& options = {});

    Image read_image(const std::filesystem::path& path);

    BMPImageInfo read_image_info(const std::filesystem::path& path);
} // namespace img::bmp 

#endif//_CPP_IMAGE_BMP_HPP_