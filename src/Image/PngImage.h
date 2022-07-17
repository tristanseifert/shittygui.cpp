#ifndef SHITTYGUI_IMAGE_PNGIMAGE_H
#define SHITTYGUI_IMAGE_PNGIMAGE_H

#include <cstddef>
#include <filesystem>
#include <vector>

#include <shittygui/Types.h>
#include <shittygui/Image.h>

namespace shittygui::image {
/**
 * @brief PNG image
 *
 * An image class that supports reading bitmaps from PNG files on the filesystem.
 *
 * @remark This class requires libpng to be available on the system.
 */
class PngImage: public Image {
    public:
        PngImage(const std::filesystem::path &path);
        ~PngImage();

        /**
         * @brief Get the PNG image surface
         *
         * This surface is read and filled with data when we load the image in the constructor.
         */
        struct _cairo_surface *getSurface() const override {
            return this->surface;
        }
        Size getSize() const override;

        static bool IsSupported();

    private:
        void doPngRead(FILE *, const size_t);

        /**
         * @brief Convert to premultiplied alpha
         */
        constexpr static inline int MultiplyAlpha(const int alpha, const int color) {
            int temp = (alpha * color) + 0x80;
            return ((temp + (temp >> 8)) >> 8);
        }

    private:
        /// The underlying Cairo surface (of image surface typoe) the image is loaded into
        struct _cairo_surface *surface{nullptr};
        /// Buffer to hold the read image bitmap data
        std::vector<std::byte> framebuffer;
};
}

#endif
