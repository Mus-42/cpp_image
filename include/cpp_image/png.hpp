#ifndef _CPP_IMAGE_PNG_HPP_
#define _CPP_IMAGE_PNG_HPP_

#include <cstdint>
#include <filesystem>

#include "image.hpp"

namespace img::png {
    enum class ColorType {
        Grayscale = 0,
        TrueColor = 2,
        Indexed = 3,
        GrayscaleAlpha = 4,
        TrueColorAlpha = 6,
    };

    // TODO options

    void write_image(const std::filesystem::path& path, const Image& img);

    Image read_image(const std::filesystem::path& path);
}

#endif//_CPP_IMAGE_PNG_HPP_