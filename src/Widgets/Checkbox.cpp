#include <cmath>

#include <cairo.h>
#include <pango/pangocairo.h>

#include "CairoHelpers.h"
#include "Util.h"
#include "Widgets/Checkbox.h"

using namespace shittygui::widgets;

/**
 * @brief Clean up allocated resources
 */
Checkbox::~Checkbox() {
    if(this->fontDesc) {
        pango_font_description_free(this->fontDesc);
        this->fontDesc = nullptr;
    }
}

/**
 * @brief Draw the checkbox component
 */
void Checkbox::drawCheck(cairo_t *drawCtx, const bool everything) {
    // get a square bounds for the check rectangle
    auto bounds = this->getBounds();
    if(bounds.size.width > bounds.size.height) {
        bounds.size.width = bounds.size.height;
    } else {
        bounds.size.height = bounds.size.width;
    }
    this->checkRect = bounds;

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
 * @brief Draw the checkbox label
 *
 * The label is drawn left aligned on the right of the checkbox.
 *
 * @remark Caller guarantees a label is set before invoking this
 */
void Checkbox::drawLabel(cairo_t *drawCtx, const bool everything) {
    // set up text rendering
    if(!this->hasTextResources()) {
        this->initTextResources(drawCtx);

        this->setTextLayoutAlign(TextAlign::Left, false);
        this->setTextLayoutWrapMode(false, true);
        this->setTextLayoutEllipsization(EllipsizeMode::End);

        if(!this->fontDesc) {
            this->setFont(kDefaultFont, kDefaultFontSize);
        }
    }

    this->updateTextLayout();

    // draw the string
    const auto offset = this->checkRect.size.width + 6;
    auto rect = this->getBounds();
    rect.origin.x += offset;
    rect.size.width -= offset;

    this->drawString(drawCtx, rect, this->textColor, VerticalAlign::Middle);
}

/**
 * @brief Update the text layout
 */
void Checkbox::updateTextLayout() {
    if(this->labelDirty) {
        this->setTextContent(*this->label, false);
        this->labelDirty = false;
    }

    if(this->fontDirty) {
        pango_layout_set_font_description(this->layout, this->fontDesc);
        this->fontDirty = false;
    }
}

/**
 * @brief Set the font used to render the checkbox label title
 *
 * @param name Font name
 * @param size Font size, in points
 */
void Checkbox::setFont(const std::string_view name, const double size) {
    if(this->fontDesc) {
        pango_font_description_free(this->fontDesc);
    }

    this->fontDesc = this->getFont(name, size);
    this->fontDirty = true;
}



/**
 * @brief Handle a touch event
 *
 * This will trigger the checkbox action when the touch is _up_ inside of our bounds; we'll also
 * toggle the value before this happens.
 *
 * Depending on the checkbox configuration, the event can fall inside the entire bounds (including
 * the label) or solely on the checkbox area.
 */
bool Checkbox::handleTouchEvent(const event::Touch &event) {
    const auto touchBounds = this->touchInsideCheckOnly ? this->checkRect : this->getBounds();
    const auto screenBounds = this->convertToScreenSpace(touchBounds);
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
