#ifndef _CPP_IMAGE_COLOR_HPP_
#define _CPP_IMAGE_COLOR_HPP_

#include <cstdint>

namespace img {
    // RGBA 64bit Color class
    class Color {
    public:
        constexpr Color() noexcept = default;
        constexpr Color(const Color&) noexcept = default;
        constexpr Color(Color&&) noexcept = default;
        constexpr Color& operator=(const Color&) noexcept = default;
        constexpr Color& operator=(Color&&) noexcept = default;

        constexpr Color(uint16_t r, uint16_t g, uint16_t b, uint16_t a = 0xFFFF) noexcept : r(r), g(g), b(b), a(a) {}

        ~Color() noexcept = default;

        uint16_t r{}, g{}, b{}, a{};
    };

    namespace colors {
        constexpr Color BLACK = Color(0, 0, 0, 0xFFFF);
        constexpr Color WHITE = Color(0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);

        constexpr Color TRANSPARENT = Color(0, 0, 0, 0);

        constexpr Color RED = Color(0xFFFF, 0, 0);
        constexpr Color GREEN = Color(0, 0xFFFF, 0);
        constexpr Color BLUE = Color(0, 0, 0xFFFF);

        constexpr Color YELLOW = Color(0xFFFF, 0xFFFF, 0);
        constexpr Color MAGNETTA = Color(0xFFFF, 0, 0xFFFF);
        constexpr Color CYAN = Color(0, 0xFFFF, 0xFFFF);
    } // namespace colors
} // namespace img

#endif//_CPP_IMAGE_COLOR_HPP_