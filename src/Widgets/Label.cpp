#include <cairo.h>
#include <pango/pangocairo.h>

#include "CairoHelpers.h"
#include "Util.h"
#include "Widgets/Label.h"

using namespace shittygui::widgets;

/**
 * @brief Free all rendering resources belonging to the label.
 */
Label::~Label() {
    // release font descriptor
    if(this->fontDesc) {
        pango_font_description_free(this->fontDesc);
        this->fontDesc = nullptr;
    }
}

/**
 * @brief Clean up the label's allocated Pango resources
 */
void Label::releaseResources() {
    this->releaseTextResources();

    // restore the default dirty flags
    this->contentDirty = true;
    this->alignDirty = true;
    this->wordWrapDirty = true;
    this->ellipsizationDirty = true;
}

/**
 * @brief Render the label text content
 */
void Label::draw(cairo_t *drawCtx, const bool everything) {
    const auto &bounds = this->getBounds();

    if(!this->hasTextResources()) {
        this->initTextResources(drawCtx);
    }

    this->updateLayout();
    this->drawString(drawCtx, bounds, this->foreground);

    Widget::draw(drawCtx, everything);
}

/**
 * @brief Updates the text layout
 *
 * Apply any changed text layout attributes to the underlying Pango layout object.
 */
void Label::updateLayout() {
    // text content
    if(this->contentDirty) {
        pango_layout_set_text(this->layout, this->content.c_str(), this->content.length());
        this->contentDirty = false;
    }

    // font
    if(this->fontDirty) {
        pango_layout_set_font_description(this->layout, this->fontDesc);
        this->fontDirty = false;
    }

    // text alignment and justification
    if(this->alignDirty) {
        this->setTextLayoutAlign(this->align, this->justified);
        this->alignDirty = false;
    }

    // word wrapping state (either word or char boundaries)
    if(this->wordWrapDirty) {
        this->setTextLayoutWrapMode(true, this->wordWrap);
        this->wordWrapDirty = false;
    }

    // ellipsization mode
    if(this->ellipsizationDirty) {
        this->setTextLayoutEllipsization(this->ellipsizationMode);
        this->ellipsizationDirty = false;
    }
}


/**
 * @brief Set the font used by the label
 *
 * Fonts are automatically loaded using the system's font discovery mechanism. Names are parsed as
 * [Pango FontDescriptions](https://docs.gtk.org/Pango/type_func.FontDescription.from_string.html)
 * so you can customize the style, variants, weight, gravity, and stretch values of the font.
 *
 * @param name Font name
 * @param size Font size, in points
 */
void Label::setFont(const std::string_view name, const double size) {
    if(this->fontDesc) {
        pango_font_description_free(this->fontDesc);
    }

    this->fontDesc = this->getFont(name, size);
    this->fontDirty = true;
}
