#include <cpp_image/exceptions.hpp>

#include "png_common.hpp"
#include <numeric>

namespace img::png::impl {
    struct BitIter {
        BitIter(const uint8_t* data, size_t size) : data_cur(data), data_end(data+size), currend_bit(0) {}

        bool is_empty() const {
            return data_cur == data_end;
        }
        
        void next() {
            currend_bit += !is_empty();
            data_cur += currend_bit >> 8;
            currend_bit &= 255;
        }

        bool get() const {
            return (*data_cur) >> currend_bit & 1;
        }

        bool get_next() {
            bool val = get();
            next();
            return val;
        }

        void next_byte() {
            currend_bit = 0;
            data_cur += !is_empty();
        }

        void eat_bytes(size_t count) {
            currend_bit = 0;
            data_cur += count;
            if (data_cur > data_end) {
                data_cur = data_end;
            }
        }

        uint8_t get_byte() const {
            return *data_cur;
        }

        uint8_t get_next_byte() {
            uint8_t byte = get_byte();
            next_byte();
            return byte;
        }

        const uint8_t* get_ptr() const {
            return data_cur;
        }

        size_t size() const {
            return data_end - data_cur;
        }

        const uint8_t* data_cur;
        const uint8_t* data_end;
        size_t currend_bit;
    };
    
    std::vector<uint8_t> dectrypt_deflate_stream(const uint8_t* data, size_t size) {
        BitIter it{data, size};
        std::vector<uint8_t> decrypted;
        bool bfinal;
        do {
            bfinal = it.get_next();
            unsigned method = it.get_next() << 1u | static_cast<unsigned>(it.get_next());
            switch (method) {
            // no compression
            case 0b00: {
                it.next_byte();
                uint16_t len = it.get_next_byte() | it.get_next_byte() << 8;
                uint16_t nlen = it.get_next_byte() | it.get_next_byte() << 8;
    
                if (len != (nlen ^ 0xFFFF)) {
                    throw PngReadError(PngReadErrorKind::INVALID_DEFLATE_CHUNK);
                }
                
                size_t rest = it.size();
                if (rest < len) {
                    throw PngReadError(PngReadErrorKind::INVALID_DEFLATE_CHUNK);
                }

                size_t at = decrypted.size();
                decrypted.reserve(std::bit_ceil(at + len));
                decrypted.resize(at + len);
                std::memcpy(decrypted.data() + at, it.get_ptr(), len);
                it.eat_bytes(len);
            } break;
            // compressed with dynamic Huffman codes
            case 0b10: {
                
            } break;
            // compressed with fixed Huffman codes
            case 0b01: {
                do {
                    // TODO
                } while(42);
            } break;
            default:
                throw PngReadError(PngReadErrorKind::INVALID_DEFLATE_CHUNK);
            }
        } while(!bfinal);

        return decrypted;
    }
} // namespace img::png::impl