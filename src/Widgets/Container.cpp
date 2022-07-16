#include <cairo.h>

#include "CairoHelpers.h"
#include "Errors.h"
#include "Util.h"
#include "Widgets/Container.h"

using namespace shittygui::widgets;

/**
 * @brief Draw the container
 *
 * We'll fill the background color and then draw an optional border.
 */
void Container::draw(cairo_t *drawCtx, const bool everything) {
    if(!this->dirtyFlag) {
        return;
    }

    // set the path to use for border and rec
    auto rect = this->getBounds();

    if(this->borderRadius > 0) {
        cairo::RoundedRect(drawCtx, rect, this->borderRadius);
    } else {
        cairo::Rectangle(drawCtx, rect);
    }

    // fill background
    cairo::SetSource(drawCtx, this->background);
    cairo_fill_preserve(drawCtx);

    // border
    if(this->drawBorder) {
        cairo::SetSource(drawCtx, this->border);

        if(this->borderRadius > 0) {
            cairo_set_line_cap(drawCtx, CAIRO_LINE_CAP_ROUND);
            cairo_set_line_join(drawCtx, CAIRO_LINE_JOIN_ROUND);
        } else {
            cairo_set_line_cap(drawCtx, CAIRO_LINE_CAP_BUTT);
            cairo_set_line_join(drawCtx, CAIRO_LINE_JOIN_BEVEL);
        }

        cairo_set_line_width(drawCtx, kBorderWidth);

        cairo_stroke(drawCtx);
    }

    Widget::draw(drawCtx, everything);
}

