#include <cmath>

#include <cairo.h>

#include "CairoHelpers.h"
#include "Util.h"
#include "Widgets/Checkbox.h"

using namespace shittygui::widgets;

/**
 * @brief Draw a regular push button
 */
void Checkbox::draw(cairo_t *drawCtx, const bool everything) {
    const auto &bounds = this->getBounds();

    cairo::RoundedRect(drawCtx, bounds, this->borderRadius);

    // draw background
    if(this->selected) {
        cairo::SetSource(drawCtx, this->selectedFillingColor);
    } else {
        cairo::SetSource(drawCtx, this->fillingColor);
    }

    cairo_fill(drawCtx);

    // draw outer stroke
    cairo::RoundedRect(drawCtx, bounds, this->borderRadius);
    cairo::SetSource(drawCtx, this->borderColor);

    cairo_set_line_cap(drawCtx, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(drawCtx, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_width(drawCtx, this->borderWidth);

    cairo_stroke(drawCtx);

    // draw checkmark
    if(this->checked) {
        const auto checkWidth = std::max(2.,
                    static_cast<double>(std::min(bounds.size.width, bounds.size.height)) * .15);
        const auto checkBounds = bounds.inset(std::floor(this->borderWidth + 4. + (checkWidth / 2.)));

        // build the path (a basic cross)
        cairo_new_path(drawCtx);
        cairo_move_to(drawCtx, checkBounds.origin.x, checkBounds.origin.y);
        cairo_line_to(drawCtx, checkBounds.origin.x + checkBounds.size.width,
                checkBounds.origin.y + checkBounds.size.height);

        cairo_move_to(drawCtx, checkBounds.origin.x + checkBounds.size.width,
                checkBounds.origin.y);
        cairo_line_to(drawCtx, checkBounds.origin.x,
                checkBounds.origin.y + checkBounds.size.height);

        // stroke it
        cairo_set_line_width(drawCtx, checkWidth);

        if(this->selected) {
            cairo::SetSource(drawCtx, this->selectedCheckColor);
        } else {
            cairo::SetSource(drawCtx, this->checkColor);
        }

        cairo_stroke(drawCtx);
    }

    Widget::draw(drawCtx, everything);
}

/**
 * @brief Handle a touch event
 *
 * This will trigger the checkbox action when the touch is _up_ inside of our bounds; we'll also
 * toggle the value before this happens.
 */
bool Checkbox::handleTouchEvent(const event::Touch &event) {
    const auto screenBounds = this->convertToScreenSpace(this->getBounds());
    const auto within = screenBounds.contains(event.position);

    this->selected = within & event.isDown;
    this->needsDisplay();

    if(within) {
        if(!event.isDown) {
            this->checked = !this->checked;

            if(this->pushCallback) {
                (*this->pushCallback)(this->shared_from_this());
            }
        }
    }

    return true;
}
