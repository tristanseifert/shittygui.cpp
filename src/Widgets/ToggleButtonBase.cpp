#include <cmath>

#include <cairo.h>
#include <pango/pangocairo.h>

#include "CairoHelpers.h"
#include "Util.h"
#include "Widgets/ToggleButtonBase.h"

using namespace shittygui::widgets;

/**
 * @brief Clean up allocated resources
 */
ToggleButtonBase::~ToggleButtonBase() {
    if(this->fontDesc) {
        pango_font_description_free(this->fontDesc);
        this->fontDesc = nullptr;
    }
}

/**
 * @brief Draw the button label
 *
 * The label is drawn left aligned on the right of the check area.
 *
 * @remark Caller guarantees a label is set before invoking this
 */
void ToggleButtonBase::drawLabel(cairo_t *drawCtx, const bool everything) {
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
void ToggleButtonBase::updateTextLayout() {
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
 * @brief Set the font used to render the label title
 *
 * @param name Font name
 * @param size Font size, in points
 */
void ToggleButtonBase::setFont(const std::string_view name, const double size) {
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
bool ToggleButtonBase::handleTouchEvent(const event::Touch &event) {
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
