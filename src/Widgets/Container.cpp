#include <algorithm>
#include <functional>
#include <stdexcept>

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
    // fill background
    cairo::SetSource(drawCtx, this->background);
    cairo_paint(drawCtx);

    // border
    if(this->drawBorder) {
        const auto &bounds = this->getFrame();

        cairo::SetSource(drawCtx, this->border);

        cairo_set_line_cap(drawCtx, CAIRO_LINE_CAP_BUTT);
        cairo_set_line_width(drawCtx, kBorderWidth);
        cairo_rectangle(drawCtx, .5, .5, bounds.size.width - kBorderWidth,
                bounds.size.height - kBorderWidth);
        cairo_stroke(drawCtx);
    }

    Widget::draw(drawCtx, everything);
}

