#ifndef SHITTYGUI_IMAGE_H
#define SHITTYGUI_IMAGE_H

#include <filesystem>
#include <memory>

#include <shittygui/Types.h>

namespace shittygui {
/**
 * @brief Bitmap image
 *
 * Base class for all bitmap/image types. Custom image types may be implemented by subclassing this
 * and implementing the required virtual methods.
 *
 * Additionally, some static helper methods are provided to use built-in loaders for common bitmap
 * formats, given that the underlying native libraries exist.
 */
class Image {
    public:
        virtual ~Image() = default;

        /**
         * @brief Get the Cairo surface for this image
         *
         * All images should be loaded to a Cairo surface. This call should return the previously
         * loaded surface.
         */
        virtual struct _cairo_surface *getSurface() const = 0;

        /**
         * @brief Get the physical size of the image
         */
        virtual Size getSize() const = 0;

        static std::shared_ptr<Image> Read(const std::filesystem::path &path);
};
}

#endif
