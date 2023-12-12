#include <cpp_image/bmp.hpp>
#include <cpp_image/exceptions.hpp>

#include <iostream>
#include <string_view>
#include <array>

#include <typeinfo>

int main() {
    try {
        std::array<std::pair<std::string_view, std::string_view>, 10> filenames = {{
            { "examples/images/i0.bmp", "out/i0_bmp_rewrited.bmp" },

            { "examples/images/i1_1bpp.bmp", "out/i1_1bpp_bmp_rewrited.bmp" },
            { "examples/images/i1_4bpp.bmp", "out/i1_4bpp_bmp_rewrited.bmp" },
            { "examples/images/i1_8bpp.bmp", "out/i1_8bpp_bmp_rewrited.bmp" },
            { "examples/images/i1_24bpp.bmp", "out/i1_24bpp_bmp_rewrited.bmp" },

            { "out/write_bmp_out_1.bmp", "out/write_bmp_out_1_bmp_rewrited.bmp" },
            { "out/write_bmp_out_4.bmp", "out/write_bmp_out_4_bmp_rewrited.bmp" },
            { "out/write_bmp_out_8.bmp", "out/write_bmp_out_8_bmp_rewrited.bmp" },
            { "out/write_bmp_out_16.bmp", "out/write_bmp_out_16_bmp_rewrited.bmp" },
            { "out/write_bmp_out_32.bmp", "out/write_bmp_out_32_bmp_rewrited.bmp" },
        }};

        for (auto [input, output] : filenames) {
            auto img = img::bmp::read_image(input);
            img::bmp::write_image(output, img);
        }

        std::cout << "SUCCESSFUL WRITED RESULT\n";
    }
    catch (img::bmp::BmpReadError& e) {
        std::cout << e.what() << std::endl;
        std::cout << static_cast<uint32_t>(e.kind) << std::endl;
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        std::cout << typeid(e).name() << std::endl;
    }
}