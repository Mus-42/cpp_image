#ifndef _CPP_IMAGE_PNG_COMMON_HPP_
#define _CPP_IMAGE_PNG_COMMON_HPP_

#include <cpp_image/png.hpp>
#include <array>

namespace img::png {
    constexpr std::array<uint8_t, 8> png_magic = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

    namespace impl {
        std::vector<uint8_t> dectrypt_deflate_stream(const uint8_t* data, size_t size);

        namespace chunks {
            // IHDR
            constexpr std::array<uint8_t, 4> ihdr_magic = { 0x49, 0x48, 0x44, 0x52 };
            // PLTE
            constexpr std::array<uint8_t, 4> plte_magic = { 0x50, 0x4C, 0x54, 0x45 };
            // IDAT
            constexpr std::array<uint8_t, 4> idat_magic = { 0x49, 0x44, 0x41, 0x54 };
            // IEND
            constexpr std::array<uint8_t, 4> iend_magic = { 0x49, 0x45, 0x4E, 0x44 };

            struct IHDRChunk {
                uint32_t width;
                uint32_t height;
                uint8_t bit_depth;
                uint8_t color_type;
                uint8_t compression_method;
                uint8_t filter_method;
                uint8_t interlace_metod;
            };
        } // namespace chunks

        // CRC32 computation: main part stealed from w3's example code
        // https://www.w3.org/TR/png-3/#D-CRCAppendix

        constexpr std::array<uint64_t, 256> crc_table = ([](){
            std::array<uint64_t, 256> table{};

            for (size_t i = 0; i < 256; i++) {
                uint64_t c = i;
                for (size_t j = 0; j < 8; j++) {
                    if (c & 1) {
                        c ^= 0x1DB710640ull;
                    }
                    c >>= 1;
                }
                table[i] = c;
            }

            return table;
        })();

        inline uint64_t update_crc(uint64_t old_crc, const uint8_t* buf, size_t len) noexcept {
            uint64_t new_crc = old_crc;

            for (size_t i = 0; i < len; i++) {
                size_t index = (new_crc ^ buf[i]) & 0xFF;
                new_crc = crc_table[index] ^ (new_crc >> 8);
            }

            return new_crc;
        }
    } // namespace impl

    struct CrcAccumulator {
        CrcAccumulator() noexcept = default;

        void append_buf(const uint8_t* buf, size_t len) noexcept {
            crc_value = impl::update_crc(crc_value, buf, len);
        }

        uint64_t get_crc() const noexcept {
            return crc_value ^ initial_value;
        }

    private:
        static constexpr uint64_t initial_value = 0xFFFFFFFF;

        uint64_t crc_value = initial_value;
    };
} // namespace img::png

#endif//_CPP_IMAGE_PNG_COMMON_HPP_