#include <cmath>

#include <cairo.h>

#include "CairoHelpers.h"
#include "Util.h"
#include "Widgets/RadioButton.h"

using namespace shittygui::widgets;

/**
 * @brief Draw the radio button's center part
 */
void RadioButton::drawCheck(cairo_t *drawCtx, const bool everything) {
    // get the bounds to draw our circle into
    auto bounds = this->getBounds().inset(std::ceil(this->borderWidth / 2.));
    if(bounds.size.width > bounds.size.height) {
        bounds.size.width = bounds.size.height;
    } else {
        bounds.size.height = bounds.size.width;
    }
    this->checkRect = bounds;

    // calculate center for circle
    double cX = bounds.origin.x, cY = bounds.origin.y;
    cX += static_cast<double>(bounds.size.width) / 2.;
    cY += static_cast<double>(bounds.size.height) / 2.;
    const double radius = static_cast<double>(bounds.size.height) / 2.;

    // draw background
    cairo_arc(drawCtx, cX, cY, radius, 0, cairo::DegreesToRadian(360));

    // draw background
    if(this->selected) {
        cairo::SetSource(drawCtx, this->selectedFillingColor);
    } else {
        cairo::SetSource(drawCtx, this->fillingColor);
    }

    cairo_fill_preserve(drawCtx);

    // draw outer stroke
    // cairo::RoundedRect(drawCtx, bounds, this->borderRadius);

    cairo::SetSource(drawCtx, this->borderColor);

    cairo_set_line_cap(drawCtx, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(drawCtx, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_width(drawCtx, this->borderWidth);

    cairo_stroke(drawCtx);

    // draw the indicator (a smaller dot)
    if(this->checked) {
        const auto dotWidth = static_cast<double>(bounds.size.width) * 0.5;

        // simply draw a circle centered at the appropriate location
        cairo_new_path(drawCtx);
        cairo_arc(drawCtx, cX, cY, dotWidth / 2., 0, cairo::DegreesToRadian(360));

        if(this->selected) {
            cairo::SetSource(drawCtx, this->selectedIndicatorColor);
        } else {
            cairo::SetSource(drawCtx, this->indicatorColor);
        }

        cairo_fill(drawCtx);
    }

    Widget::draw(drawCtx, everything);
}
