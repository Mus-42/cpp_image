#include <cpp_image/bmp.hpp>
#include <cpp_image/exceptions.hpp>

#include <iostream>
#include <cstdlib>

int main(int argc, const char** argv) {
    if (argc != 3 || std::string_view(argv[1]) != "-i") {
        std::cout << R"(
            Usage:
            bmp_info -i input_file
        )";
        return EXIT_FAILURE;
    }

    const char* input = argv[2];
    
    try {
        auto info = img::bmp::read_image_info(input);

        std::cout << "file: `" << input << "`" << std::endl;
        std::cout << "bpp: " << static_cast<uint32_t>(info.bpp) << std::endl;   
        std::cout << "width: " << info.width << std::endl;   
        std::cout << "height: " << info.height << std::endl;   
        std::cout << "x ppm: " << info.x_ppm << std::endl;   
        std::cout << "y ppm: " << info.y_ppm << std::endl;   
        std::cout << "file size: " << info.file_size << std::endl;   
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}