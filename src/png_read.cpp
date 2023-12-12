#include <cpp_image/png.hpp>
#include <cpp_image/exceptions.hpp>

#include "png_common.hpp"

#include <fstream>
#include <iostream>
#include <numeric>

namespace img::png {
    namespace impl {
        template<typename T>
        inline T read_int(std::ifstream& in) {
            std::array<char, sizeof(T)> buf;
            in.read(buf.data(), sizeof(T));

            T num = 0;
            for (char b : buf) {
                num = (num << 8) | static_cast<uint8_t>(b);
            }

            return num;
        }
    }

    Image read_image(const std::filesystem::path& path) {
        std::ifstream in(path, std::ios::binary);
        if (!in.is_open() || in.bad()) {
            throw PngReadError(PngReadErrorKind::CANT_OPEN_FILE);
        }

        std::array<uint8_t, 8> magic{};
        in.read(reinterpret_cast<char*>(magic.data()), 8);

        if (magic != png_magic) {
            throw PngReadError(PngReadErrorKind::INVALID_MAGIC);
        }

        std::vector<uint8_t> image_data;

        while (!in.eof()) {
            uint32_t length = impl::read_int<uint32_t>(in);
            std::array<uint8_t, 4> chunk_type;
            in.read(reinterpret_cast<char*>(chunk_type.data()), 4);

            std::cout << "Chunk: " << std::string_view(reinterpret_cast<char*>(chunk_type.data()), 4) << "\n";

            if (in.eof()) {
                throw PngReadError(PngReadErrorKind::UNEXPECTED_EOF);
            }

            size_t at = image_data.size();
            image_data.reserve(std::bit_ceil(at + length));
            image_data.resize(at + length);

            in.read(reinterpret_cast<char*>(image_data.data()) + at, length);

            if (in.eof()) {
                throw PngReadError(PngReadErrorKind::UNEXPECTED_EOF);
            }

            uint32_t crc_stored = impl::read_int<uint32_t>(in);

            CrcAccumulator accum;
            accum.append_buf(chunk_type.data(), chunk_type.size());
            accum.append_buf(image_data.data() + at, length);

            uint32_t crc_computed = static_cast<uint32_t>(accum.get_crc());

            if (crc_stored != crc_computed) {
                throw PngReadError(PngReadErrorKind::INVALID_CHUNK_CRC);
            }

            if (chunk_type == impl::chunks::iend_magic) {
                break;
            }
        }

        return {};
    }
}