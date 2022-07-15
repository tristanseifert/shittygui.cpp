#ifndef SHITTYGUI_SCREEN_H
#define SHITTYGUI_SCREEN_H

#include <cstddef>
#include <cstdint>
#include <span>

#include <shittygui/Types.h>

namespace shittygui {
/**
 * @brief GUI screen class
 *
 * Screens serve as the base of the GUI library. They accumulate inputs from various sources,
 * handle the lifecycle of controls, and draw controls (as needed, based on their dirty flags) into
 * their underlying framebuffer.
 */
class Screen {
    public:
        enum class PixelFormat {
            /// 24-bit color in a 32-bit value; upper 8 bits alpha, premultiplied
            ARGB32,
            /// 24-bit color in a 32-bit value; upper 8 bits unused
            RGB24,
            /// 16-bit color, split 5-6-5 between RGB
            RGB16,
            /// 30-bit color, 10 bits per color channel
            RGB30,
        };

        Screen(const PixelFormat format, const Size &size);
        Screen(const PixelFormat format, const Size &size, std::span<std::byte> framebuffer,
                const size_t stride);
        ~Screen();

        /// Get the physical dimensions of the underlying framebuffer
        inline constexpr auto getFramebufferSize() {
            return this->physSize;
        }
        /// Get the dimensions of the screen
        inline constexpr auto getSize() const {
            return this->size;
        }
        /// Get the screen's pixel format
        inline constexpr auto getPixelFormat() const {
            return this->format;
        }
        void *getBuffer();
        size_t getBufferStride() const;

        static size_t OptimalStrideForBuffer(const PixelFormat format, const uint16_t width);
        inline static size_t OptimalStrideForBuffer(const PixelFormat format, const Size &size) {
            return OptimalStrideForBuffer(format, size.width);
        }

    private:
        /// Pixel format of the screen
        PixelFormat format;

        /// Physical size of the output framebuffer
        Size physSize;
        /// Dimensions of the rendering surface (takes into account rotation)
        Size size;

        /// Underlying Cairo rendering surface
        struct _cairo_surface *surface{nullptr};
};
}

#endif
