#include <array>
#include <cstddef>
#include <cerrno>
#include <cstdio>
#include <system_error>
#include <vector>

#include <cairo.h>
#include <png.h>

#include "CairoHelpers.h"
#include "Errors.h"
#include "PngImage.h"

using namespace shittygui::image;

/**
 * @brief Query whether the PNG reader is supported
 *
 * This is based on a compile definition that indicates whether libpng is present.w
 */
bool PngImage::IsSupported() {
    // TODO: implement this
    return true;
}

/**
 * @brief Load a PNG image from disk
 *
 * @param path Path to the image
 */
PngImage::PngImage(const std::filesystem::path &path) {
    std::array<std::byte, 8> header;

    // open file
    auto fp = fopen(path.native().c_str(), "rb");
    if(!fp) {
        throw std::system_error(errno, std::generic_category(), "fopen image");
    }

    fread(header.data(), 1, header.size(), fp);
    if(png_sig_cmp(reinterpret_cast<png_bytep>(header.data()), 0, header.size())) {
        fclose(fp);
        throw std::invalid_argument("file is not a png");
    }

    // cool, it's a PNG, so read it
    try {
        this->doPngRead(fp, header.size());
    } catch(const std::exception &) {
        fclose(fp);
        throw;
    }

    fclose(fp);
}

/**
 * @brief Read an opened PNG file
 *
 * @param fp File pointer to the opened PNG file
 * @param sigBytesRead Number of signature bytes that were read
 */
void PngImage::doPngRead(FILE *fp, const size_t sigBytesRead) {
    // set up for reading
    auto pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if(!pngPtr) {
        throw std::runtime_error("png_create_read_struct failed");
    }

    auto infoPtr = png_create_info_struct(pngPtr);
    if(!infoPtr) {
        throw std::runtime_error("png_create_info_struct failed");
    }

    png_init_io(pngPtr, fp);
    png_set_sig_bytes(pngPtr, sigBytesRead);

    // then fetch the actual image information
    png_read_info(pngPtr, infoPtr);

    const auto width = png_get_image_width(pngPtr, infoPtr),
          height = png_get_image_height(pngPtr, infoPtr);

    // currently, only RGB/RGBA images with 8 bit channel depth are supported
    const auto type = png_get_color_type(pngPtr, infoPtr);
    if(type != PNG_COLOR_TYPE_RGB && type != PNG_COLOR_TYPE_RGBA) {
        throw std::invalid_argument("unsupported color type (only RGB/RGBA currently supported)");
    }

    const auto bpc = png_get_bit_depth(pngPtr, infoPtr);
    if(bpc != 8) {
        throw std::invalid_argument("unsupported channel depth (only 8 bpc currently supported)");
    }

    const auto numPasses = png_set_interlace_handling(pngPtr);
    png_read_update_info(pngPtr, infoPtr);

    // create a surface
    this->surface = cairo_image_surface_create(
            (type == PNG_COLOR_TYPE_RGB) ? CAIRO_FORMAT_RGB24 : CAIRO_FORMAT_ARGB32, width,height);
    auto status = cairo_surface_status(this->surface);

    if(status != CAIRO_STATUS_SUCCESS) {
        ThrowForCairoStatus(status);
    }

    // set up row pointers into the surface
    std::vector<void *> rowPtrs;
    rowPtrs.resize(height, nullptr);

    auto surfaceBase = cairo_image_surface_get_data(this->surface);
    const auto surfaceStride = cairo_image_surface_get_stride(this->surface);

    for(size_t y = 0; y < height; y++) {
        rowPtrs[y] = surfaceBase + (surfaceStride * y);
    }

    png_read_image(pngPtr, reinterpret_cast<png_bytep *>(rowPtrs.data()));

    // clean up
    png_destroy_read_struct(&pngPtr, &infoPtr, nullptr);
}

/**
 * @brief Release all resources
 */
PngImage::~PngImage() {
    if(this->surface) {
        cairo_surface_destroy(this->surface);
    }
}

/**
 * @brief Get the physical pixel size of the image
 */
shittygui::Size PngImage::getSize() const {
    auto w = cairo_image_surface_get_width(this->surface),
         h = cairo_image_surface_get_height(this->surface);

    return Size(w, h);
}

