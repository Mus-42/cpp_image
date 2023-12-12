#include <cpp_image/bmp.hpp>
#include <iostream>

int main() {
    img::Image img(1024, 1024);
    for (uint32_t i = 0; i < 1024; i++) {
        for (uint32_t j = 0; j < 1024; j++) {
            uint16_t r = static_cast<uint16_t>(i)<<6;
            uint16_t g = static_cast<uint16_t>(j)<<6;
            uint16_t b = static_cast<uint16_t>(((1024 - i) * (1024 - i) + (1024 - j) * (1024 - j))>>5);

            img[{i, j}] = img::Color(r, g, b);
        }
    }

    using img::bmp::BitsPerPixel;
    static constexpr auto all_bpps = {
        BitsPerPixel::One,
        BitsPerPixel::Four,
        BitsPerPixel::Eight,
        BitsPerPixel::Sixteen,
        BitsPerPixel::ThirtyTwo,
    };

    for (auto bpp : all_bpps) {
        auto path = "out/write_bmp_out_" + std::to_string(static_cast<uint8_t>(bpp)) + ".bmp";
        std::cout << path << '\n';
        img::bmp::write_image(path, img, { .bpp = bpp });
    }
}