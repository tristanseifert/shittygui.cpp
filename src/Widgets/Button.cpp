#include <cairo.h>
#include <pango/pangocairo.h>

#include "CairoHelpers.h"
#include "Util.h"
#include "Widgets/Button.h"

using namespace shittygui::widgets;

/**
 * @brief Free all rendering resources belonging to the button.
 */
Button::~Button() {
    if(this->fontDesc) {
        pango_font_description_free(this->fontDesc);
        this->fontDesc = nullptr;
    }
}

/**
 * @brief Draw a regular push button
 */
void Button::drawPushButton(cairo_t *drawCtx, const bool everything) {
    const auto &bounds = this->getBounds();

    // draw filling
    const auto fillingBounds = bounds.inset(this->borderWidth);

    cairo::RoundedRect(drawCtx, fillingBounds, this->borderRadius);

    if(this->selected) {
        cairo::SetSource(drawCtx, this->selectedFillingColor);
    } else {
        cairo::SetSource(drawCtx, this->fillingColor);
    }

    cairo_fill(drawCtx);

    // draw border
    cairo::RoundedRect(drawCtx, bounds, this->borderRadius);
    cairo::SetSource(drawCtx, this->borderColor);

    cairo_set_line_cap(drawCtx, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_join(drawCtx, CAIRO_LINE_JOIN_MITER);
    cairo_set_line_width(drawCtx, this->borderWidth);

    cairo_stroke(drawCtx);

    // draw text
    if(!this->hasTextResources()) {
        this->initTextResources(drawCtx);

        this->setTextLayoutAlign(TextAlign::Center, false);
        this->setTextLayoutWrapMode(false, false);
        this->setTextLayoutEllipsization(EllipsizeMode::Middle);

        if(!this->fontDesc) {
            this->setFont(kDefaultFont, kDefaultFontSize);
        }
    }

    this->updateTextLayout();

    if(this->selected) {
        this->drawString(drawCtx, fillingBounds, this->selectedTextColor);
    } else {
        this->drawString(drawCtx, fillingBounds, this->textColor, VerticalAlign::Middle);
    }
}

/**
 * @brief Update the text layout
 */
void Button::updateTextLayout() {
    if(this->titleDirty) {
        pango_layout_set_text(this->layout, this->title.c_str(), this->title.length());
        this->titleDirty = false;
    }

    if(this->fontDirty) {
        pango_layout_set_font_description(this->layout, this->fontDesc);
        this->fontDirty = false;
    }
}

/**
 * @brief Set the font used to render the button title
 *
 * @param name Font name
 * @param size Font size, in points
 */
void Button::setFont(const std::string_view name, const double size) {
    if(this->fontDesc) {
        pango_font_description_free(this->fontDesc);
    }

    this->fontDesc = this->getFont(name, size);
    this->fontDirty = true;
}
