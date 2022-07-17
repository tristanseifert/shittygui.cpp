#include <cstdio>

#include <cairo.h>

#include "PngImage.h"
#include "Image.h"

using namespace shittygui;

/**
 * @brief Read an image from disk
 *
 * Attempt to read an image from disk using built-in image readers. The following image formats
 * are supported:
 *
 * - PNG: Required libpng present on the system.
 *
 * @param path File path of the image
 *
 * @return Image instance
 *
 * @throws std::invalid_argument File does not exist
 * @throws std::runtime_error If the image could not be loaded
 */
std::shared_ptr<Image> Image::Read(const std::filesystem::path &path) {
    // make sure file exists
    if(!std::filesystem::exists(path)) {
        throw std::invalid_argument("file does not exist");
    }

    // try as PNG
    try {
        auto png = std::make_shared<image::PngImage>(path);
        return png;
    } catch(const std::exception &e) {
        fprintf(stderr, "failed to read image '%s' as PNG: %s\n", path.native().c_str(), e.what());
    }

    // failed to read the image
    throw std::runtime_error("unsupported image format");
}

