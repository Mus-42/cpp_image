#ifndef _CPP_IMAGE_EXCEPTIONS_HPP_
#define _CPP_IMAGE_EXCEPTIONS_HPP_

#include <stdexcept>
#include <array>

namespace img {
    class ImgException: public std::runtime_error {
    public:
        ImgException(const char* what_arg) : std::runtime_error(what_arg) {}
    };

    class OutOfRage: public ImgException {
    public:
        OutOfRage(const char* what_arg) : ImgException(what_arg) {}
    };
    

    namespace bmp {
        enum class BmpReadErrorKind: uint8_t {
            CANT_OPEN_FILE,
            INVALID_MAGIC,
            UNEXPECTED_EOF,
            UNSUPPORTED_BITMAP_HEADER,
            INVALID_HEADER_DATA,

            __TOTAL_ERRORS_COUNT
        };

        namespace impl {
            constexpr std::array<const char*, static_cast<size_t>(BmpReadErrorKind::__TOTAL_ERRORS_COUNT)> bmp_read_error = {
                "BMP Read error: can't open file",
                "BMP Read error: invalid bmp magic",
                "BMP Read error: unexpected EOF",
                "BMP Read error: unsupported bitmap header (expected BITMAPINFOHEADER v1 v3 v4 or v5)"
                "BMP Read error: invalid (or unsupported) values in bitmap header"
            };

        } // namespace impl

        
        class BmpReadError: public ImgException {
        public:
            BmpReadError(BmpReadErrorKind kind) : ImgException(impl::bmp_read_error[static_cast<size_t>(kind)]), kind(kind) {}

            const BmpReadErrorKind kind;

            virtual ~BmpReadError() noexcept = default; 
        };
    } // namespace bmp

    namespace png {
        enum class PngReadErrorKind: uint8_t {
            CANT_OPEN_FILE,
            INVALID_MAGIC,
            INVALID_CHUNK_CRC,
            UNEXPECTED_EOF,
            INVALID_DEFLATE_CHUNK,

            __TOTAL_ERRORS_COUNT
        };

        namespace impl {
            constexpr std::array<const char*, static_cast<size_t>(PngReadErrorKind::__TOTAL_ERRORS_COUNT)> png_read_error = {
                "PNG Read error: can't open file",
                "PNG Read error: invalid png magic",
                "PNG Read error: invalid chunk CRC",
                "PNG Read error: unexpected EOF",
                "PNG Read error: invalid Deflate chunk",
            };

        } // namespace impl

        
        class PngReadError: public ImgException {
        public:
            PngReadError(PngReadErrorKind kind) : ImgException(impl::png_read_error[static_cast<size_t>(kind)]), kind(kind) {}

            const PngReadErrorKind kind;

            virtual ~PngReadError() noexcept = default; 
        };
    } // namespace png
} // namespace img

#endif//_CPP_IMAGE_EXCEPTIONS_HPP_