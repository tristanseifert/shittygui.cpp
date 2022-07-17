#ifndef SHITTYGUI_IMAGE_PNGIMAGE_H
#define SHITTYGUI_IMAGE_PNGIMAGE_H

#include <cstddef>
#include <filesystem>

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

    private:
        /// The underlying Cairo surface (of image surface typoe) the image is loaded into
        struct _cairo_surface *surface{nullptr};
};
}

#endif
