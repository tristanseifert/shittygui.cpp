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

    // draw icon
    if(this->shouldRenderIcon && this->icon) {
        this->drawIcon(drawCtx, fillingBounds);
    }

    // draw text
    if(this->shouldRenderTitle && !this->title.empty()) {
        this->drawTitle(drawCtx, fillingBounds);
    }
}

/**
 * @brief Draw the button title
 *
 * Call this after the icon (if any) has been rendered. Icons will always render below text, and
 * the text drawing routine relies on knowing where the icon was drawn.
 *
 * @param drawCtx Drawing context to render on
 * @param contentRect Rectangle for the content area
 */
void Button::drawTitle(cairo_t *drawCtx, const Rect &contentRect) {
    // inset the text rectangle, if the icon is on the left or right side
    auto rect = contentRect;

    if(this->shouldRenderIcon && this->icon) {
        switch(this->ig) {
            case IconGravity::Left:
                rect.size.width -= this->iconRect.size.width + this->iconPadding;
                rect.origin.x += this->iconRect.size.width + this->iconPadding;
                break;

            case IconGravity::Right:
                rect.size.width -= this->iconRect.size.width + this->iconPadding;
                break;

            // for center icons, don't do anything; you shouldn't have a title here anyways
            case IconGravity::Center:
                break;
        }
    }

    // create text layout if needed
    if(!this->hasTextResources()) {
        this->initTextResources(drawCtx);

        this->setTextLayoutAlign(TextAlign::Center, false);
        this->setTextLayoutWrapMode(false, true);
        this->setTextLayoutEllipsization(EllipsizeMode::Middle);

        if(!this->fontDesc) {
            this->setFont(kDefaultFont, kDefaultFontSize);
        }
    }

    this->updateTextLayout();

    // draw string
    if(this->selected) {
        this->drawString(drawCtx, rect, this->selectedTextColor, VerticalAlign::Middle);
    } else {
        this->drawString(drawCtx, rect, this->textColor, VerticalAlign::Middle);
    }
}

/**
 * @brief Draw the button's icon.
 *
 * The icon will always be vertically centered; its horizontal alignment is specified by the icon
 * gravity value. We'll scale the icon to be either its full size, or such that it has a fixed
 * amount of padding between the top and bottom of the content region.
 *
 * @param drawCtx Drawing context to render into
 * @param contentRect Rect for the content of the button
 */
void Button::drawIcon(cairo_t *drawCtx, const Rect &contentRect) {
    // calculate the rect for the icon
    const auto &iconSize = this->icon->getSize();
    auto iconRect = contentRect.inset(this->iconPadding);
    const auto iconSpaceWidth = iconRect.size.width;

    if(iconRect.size.height > iconSize.height) {
        const auto diff = iconRect.size.height - iconSize.height;
        iconRect.size.height = iconSize.height;
        iconRect.origin.y += diff / 2;
    }

    const auto ratio = static_cast<double>(iconSize.height) / static_cast<double>(iconSize.width);
    iconRect.size.width = iconRect.size.height * ratio;

    switch(this->ig) {
        case IconGravity::Left:
            break;
        case IconGravity::Center:
            iconRect.origin.x += (iconSpaceWidth / 2) - (iconRect.size.width / 2);
            break;
        case IconGravity::Right:
            iconRect.origin.x += iconSpaceWidth - iconRect.size.width;
            break;
    }

    // draw the icon
    this->iconRect = iconRect;
    cairo::Rectangle(drawCtx, iconRect);

    const double iconScale = static_cast<double>(iconRect.size.height) / static_cast<double>(iconSize.height);
    cairo_scale(drawCtx, iconScale, iconScale);

    cairo_set_source_surface(drawCtx, this->icon->getSurface(), iconRect.origin.x, iconRect.origin.y);
    cairo_fill(drawCtx);
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

    if(this->iconGravityDirty) {
        switch(this->ig) {
            case IconGravity::Left:
                this->setTextLayoutAlign(TextAlign::Left, false);
                break;
            case IconGravity::Center:
                this->setTextLayoutAlign(TextAlign::Center, false);
                break;
            case IconGravity::Right:
                this->setTextLayoutAlign(TextAlign::Right, false);
                break;
        }
        this->iconGravityDirty = false;
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

/**
 * @brief Handle a touch event
 *
 * This will trigger the button's action when the touch is _up_ inside of the button's bounds.
 */
bool Button::handleTouchEvent(const event::Touch &event) {
    const auto screenBounds = this->convertToScreenSpace(this->getBounds());
    const auto within = screenBounds.contains(event.position);

    this->selected = within & event.isDown;
    this->needsDisplay();

    if(within) {
        if(!event.isDown) {
            if(auto cb = *this->pushCallback) {
                cb(this->shared_from_this());
            }
        }
    }

    return true;
}
