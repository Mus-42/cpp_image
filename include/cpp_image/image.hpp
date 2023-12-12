#ifndef _CPP_IMAGE_IMAGE_HPP_
#define _CPP_IMAGE_IMAGE_HPP_

#include <cstdint>
#include <memory>
#include <array>
#include <utility>
#include <cassert>

#include "color.hpp"

namespace img {
    class Image {
    public:
        Image() noexcept = default;
        
        // only explicit copy
        explicit Image(const Image&) noexcept = default;
        Image& operator=(const Image&) noexcept = delete;

        Image(Image&&) noexcept = default;
        Image& operator=(Image&&) noexcept = default;
        
        ~Image() noexcept = default;

        Image(uint32_t width, uint32_t height) noexcept : m_width(width), m_height(height), 
            m_pixels(std::make_unique<Color[]>(static_cast<size_t>(width * height))) 
        {
        }

        std::pair<uint32_t, uint32_t> dimensions() const noexcept {
            return { m_width, m_height };
        }

        Color& operator[](std::pair<uint32_t, uint32_t> index) noexcept {
            const auto [i, j] = index;
            size_t pixels_index = static_cast<size_t>(i * m_width + j);
            return m_pixels[pixels_index];
        }

        const Color& operator[](std::pair<uint32_t, uint32_t> index) const noexcept {
            const auto [i, j] = index;
            size_t pixels_index = static_cast<size_t>(i * m_width + j);
            return m_pixels[pixels_index];
        }
    private:
        uint32_t m_width, m_height;
        std::unique_ptr<Color[]> m_pixels;
    };
} // namespace img

#endif//_CPP_IMAGE_IMAGE_HPP_