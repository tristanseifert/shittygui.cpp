#include <cmath>
#include <chrono>

#include <cairo.h>

#include "CairoHelpers.h"
#include "Errors.h"
#include "Util.h"
#include "Widgets/ProgressBar.h"

using namespace shittygui::widgets;

/**
 * @brief Clean up all allocated resources
 *
 * This will terminate any animations, and release allocated patterns for the fill of the bar.
 */
void ProgressBar::releaseResources() {
    if(this->barberSurface) {
        cairo_surface_destroy(this->barberSurface);
    }
    if(this->barberPattern) {
        cairo_pattern_destroy(this->barberPattern);
    }
}

/**
 * @brief Draw the progress bar
 *
 * First, draw the common outer border for the bar. Then, depending on the style, either apply the
 * solid proportional filling, or the animated content filling.
 */
void ProgressBar::draw(cairo_t *drawCtx, const bool everything) {
    const auto &bounds = this->getBounds();

    // draw border
    cairo::Rectangle(drawCtx, bounds);

    cairo::SetSource(drawCtx, kBorderColor);

    cairo_set_line_cap(drawCtx, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join(drawCtx, CAIRO_LINE_JOIN_MITER);
    cairo_set_line_width(drawCtx, kBorderWidth);

    cairo_stroke(drawCtx);

    // calculate the filling's rect
    const auto fillingRect = bounds.inset(kBorderWidth);

    // draw the solid filled style
    if(this->style == Style::Determinate) {
        const double filledWidth = static_cast<double>(fillingRect.size.width) * this->progress;

        // draw the filled part of the bar
        if(this->progress > 0) {
            cairo_rectangle(drawCtx, fillingRect.origin.x, fillingRect.origin.y, filledWidth,
                    fillingRect.size.height);

            cairo_set_source_rgb(drawCtx, 0.7, 0.7, 1.);
            cairo_fill(drawCtx);
        }

        // draw the unfilled part
        cairo_rectangle(drawCtx, std::floor(filledWidth), fillingRect.origin.y,
                std::ceil(static_cast<double>(fillingRect.size.width) - filledWidth) + 1,
                fillingRect.size.height);

        cairo_set_source_rgb(drawCtx, 0.2, 0.2, 0.4);
        cairo_fill(drawCtx);
    }
    // handle the animated indeterminate style
    else {
        // update the animation pattern
        if(!this->barberPattern || this->fillDirty) {
            this->updateIndeterminateFill(fillingRect);
            this->fillDirty = false;
        }

        /*
         * Figure out the horizontal offset to use with the pattern
         *
         * The period of one full "revolution" of this pattern should be the same for all bars,
         * regardless of their height. Calculate a required speed in pixels per second to get the
         * appropriate interval, and use the current time to animate that.
         */
        const auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
        const auto usec = std::chrono::duration_cast<std::chrono::microseconds>(now).count();
        const auto intervalUsec = static_cast<uint64_t>(kIndeterminateAnimInterval * 1000. * 1000.);
        const double offsetPercent = static_cast<double>(usec % intervalUsec) /
            static_cast<double>(intervalUsec);

        // apply the transformation to the pattern
        cairo_matrix_t matrix;
        cairo_matrix_init_translate(&matrix, offsetPercent * this->patternWidth, -kBorderWidth);
        cairo_pattern_set_matrix(this->barberPattern, &matrix);

        // then draw it
        cairo_set_source(drawCtx, this->barberPattern);

        cairo::Rectangle(drawCtx, fillingRect);
        cairo_fill(drawCtx);
    }

    Widget::draw(drawCtx, everything);
}

/**
 * @brief Render the indeterminate fill pattern
 *
 * Draw into a surface-backed pattern the barber pole pattern for the indeterminate progress bar.
 * We'll render a square texture, the side length equal to the filling height of the bar.
 *
 * @param fillingRect Rect used for the filling of the bar
 */
void ProgressBar::updateIndeterminateFill(const Rect &fillingRect) {
    cairo_status_t status;

    // release old pattern and surfaces
    if(this->barberSurface) {
        cairo_surface_destroy(this->barberSurface);
    }
    if(this->barberPattern) {
        cairo_pattern_destroy(this->barberPattern);
    }

    // create the surface
    const double w = fillingRect.size.height * 2, h = fillingRect.size.height;
    this->patternWidth = w;

    this->barberSurface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    status = cairo_surface_status(this->barberSurface);

    if(status != CAIRO_STATUS_SUCCESS) {
        ThrowForCairoStatus(status);
    }

    // set up temporary drawing context and draw the pattern
    auto ctx = cairo_create(this->barberSurface);
    status = cairo_status(ctx);
    if(status != CAIRO_STATUS_SUCCESS) {
        ThrowForCairoStatus(status);
    }

    cairo_set_antialias(ctx, CAIRO_ANTIALIAS_BEST);

    this->drawIndeterminatePattern(ctx, w, h);

    // finalize the drawing context and release it
    cairo_surface_flush(this->barberSurface);
    cairo_destroy(ctx);

    // create a pattern for the surface once we're done drawing
    this->barberPattern = cairo_pattern_create_for_surface(this->barberSurface);
    status = cairo_pattern_status(this->barberPattern);
    if(status != CAIRO_STATUS_SUCCESS) {
        ThrowForCairoStatus(status);
    }

    cairo_pattern_set_extend(this->barberPattern, CAIRO_EXTEND_REPEAT);
}

/**
 * @brief Draw the indeterminate bar pattern
 *
 * This is a diagonal bar, which can be tiled.
 */
void ProgressBar::drawIndeterminatePattern(cairo_t *ctx, const double width, const double height) {
    // draw background
    cairo_set_source_rgb(ctx, 0, 0, 0);
    cairo_paint(ctx);

    // draw the line
    cairo_move_to(ctx, 0, 0);
    cairo_line_to(ctx, width / 2.33, 0);
    cairo_line_to(ctx, width, height);
    cairo_line_to(ctx, width - (width / 2.33), height);
    cairo_line_to(ctx, 0, 0);
    cairo_close_path(ctx);

    cairo_set_source_rgb(ctx, 0, 0, .66);
    cairo_fill(ctx);
}


/**
 * @brief Handle an animation frame
 *
 * If the bar is indeterminate, we'll simply request to get redrawn.
 */
void ProgressBar::processAnimationFrame() {
    if(this->style != Style::Indeterminate) {
        return;
    }

    this->needsDisplay();
}
