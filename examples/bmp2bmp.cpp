#include <cpp_image/bmp.hpp>
#include <cpp_image/exceptions.hpp>

#include <iostream>
#include <cstdlib>

int main(int argc, const char** argv) {
    // TODO add default args and multiple files at once

    if (argc != 7 || std::string_view(argv[1]) != "-i" || std::string_view(argv[3]) != "-o" || std::string_view(argv[5]) != "-bpp") {
        std::cout << R"(
            Usage:
            bmp2bmp -i input_file -o output_file -bpp output_bpp
        )";
        return EXIT_FAILURE;
    }

    const char* input = argv[2];
    const char* output = argv[4];
    const char* bpp_str = argv[6];

    unsigned long bpp = std::strtoul(bpp_str, nullptr, 10);
    
    if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 16 && bpp != 32) {
        std::cout << R"(
            invalid bpp specified: only 1, 4, 8, 16, 32 supported for now
        )";
        return EXIT_FAILURE;
    }

    try {
        auto img = img::bmp::read_image(input);
        img::bmp::write_image(output, img, { static_cast<img::bmp::BitsPerPixel>(bpp) });
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}