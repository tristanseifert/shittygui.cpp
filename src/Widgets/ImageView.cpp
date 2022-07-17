#include <cmath>

#include <cairo.h>

#include "CairoHelpers.h"
#include "Util.h"
#include "Widgets/ImageView.h"

using namespace shittygui::widgets;

/**
 * @brief Render an image view
 *
 * Information on how to render the image (essentially, the required set of transforms to scale
 * the image as requested) is cached based on the frame and image gravity settings.
 */
void ImageView::draw(cairo_t *drawCtx, const bool everything) {
    const auto &bounds = this->getBounds();

    // draw background
    cairo::Rectangle(drawCtx, bounds);

    cairo::SetSource(drawCtx, this->backgroundColor);
    cairo_fill(drawCtx);

    // draw the image
    if(this->image) {
        const auto imageRect = bounds.inset(std::floor(this->borderWidth));
        this->drawImage(drawCtx, imageRect);
    }

    // draw border (over the image if it peeks out at the edges)
    if(this->borderWidth > 0) {
        cairo::SetSource(drawCtx, this->borderColor);
        cairo::Rectangle(drawCtx, bounds);

        cairo_set_line_cap(drawCtx, CAIRO_LINE_CAP_BUTT);
        cairo_set_line_join(drawCtx, CAIRO_LINE_JOIN_BEVEL);
        cairo_set_line_width(drawCtx, this->borderWidth);
        cairo_stroke(drawCtx);
    }

    Widget::draw(drawCtx, everything);
}

/**
 * @brief Render the image
 *
 * Draw the image stored in the class according to the current scale information.
 *
 * @param drawCtx Cairo drawing context
 * @param imageAreaRect Rect for the total area available for the image to be drawn into
 *
 * @seeAlso updateImageTransform
 */
void ImageView::drawImage(cairo_t *drawCtx, const Rect &imageAreaRect) {
    cairo_save(drawCtx);

    // update image state if needed
    if(this->imageMatrixDirty) {
        this->updateImageTransform(imageAreaRect);
        this->imageMatrixDirty = false;
    }

    // draw a debug outline, if enabled
    cairo::Rectangle(drawCtx, this->imageRect);

    if(kDrawImageOutline) {
        cairo_set_line_width(drawCtx, 1);
        cairo_set_source_rgb(drawCtx, 1, 0, 1);
        cairo_stroke_preserve(drawCtx);
    }

    // draw the image (scaled)
    cairo_scale(drawCtx, this->imageXScale, this->imageYScale);

    cairo_set_source_surface(drawCtx, this->image->getSurface(), this->imageRect.origin.x,
            this->imageRect.origin.y);
    cairo_fill(drawCtx);

    cairo_restore(drawCtx);
}

/**
 * @brief Recalculate the image transformation
 *
 * Determine what horizontal/vertical scale factors to apply to the image, and where its origin
 * should lie for the current image gravity setting.
 *
 * @param imageAreaRect Rect for the total area available for the image to be drawn into
 */
void ImageView::updateImageTransform(const Rect &imageAreaRect) {
    const auto &origImageSize = this->image->getSize();
    Size imageSize;
    auto rect = imageAreaRect;

    // figure out the size of the image
    switch(this->imageMode) {
        // use the original image size
        case Mode::None:
            imageSize = this->image->getSize();
            this->imageXScale = 1;
            this->imageYScale = 1;
            break;

        // scale axes independently
        case Mode::ScaleIndependently:
            imageSize = imageAreaRect.size;
            this->imageXScale = static_cast<double>(imageSize.width) / static_cast<double>(origImageSize.width);
            this->imageYScale = static_cast<double>(imageSize.height) / static_cast<double>(origImageSize.height);
            break;

        // scale proportionally down
        case Mode::ScaleProportionalDown: {
            std::pair<double, double> newSize;
            const Size minImageSize = {
                std::min(origImageSize.width, imageAreaRect.size.width),
                std::min(origImageSize.height, imageAreaRect.size.height),
            };
            GetProportionalFit(origImageSize, minImageSize, newSize);

            this->imageXScale = std::floor(newSize.first + 1.) / static_cast<double>(origImageSize.width);
            this->imageYScale = std::floor(newSize.second + 1.) / static_cast<double>(origImageSize.height);

            imageSize = Size(std::ceil(newSize.first), std::ceil(newSize.second));
            break;
        }

        // scale proportionally up or down
        case Mode::ScaleProportionalUpDown: {
            std::pair<double, double> newSize;
            GetProportionalFit(origImageSize, imageAreaRect.size, newSize);

            this->imageXScale = std::floor(newSize.first) / static_cast<double>(origImageSize.width);
            this->imageYScale = std::floor(newSize.second) / static_cast<double>(origImageSize.height);

            imageSize = Size(std::ceil(newSize.first), std::ceil(newSize.second));
            break;
        }
    }

    // center the image (if there's any space left over)
    rect.size = imageSize;

    rect.origin.x += (imageAreaRect.size.width / 2) - (imageSize.width / 2);
    rect.origin.y += (imageAreaRect.size.height / 2) - (imageSize.height / 2);

    this->imageRect = rect;
}
