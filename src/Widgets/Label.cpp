#include <cairo.h>
#include <pango/pangocairo.h>

#include "CairoHelpers.h"
#include "Util.h"
#include "Widgets/Label.h"

using namespace shittygui::widgets;

/**
 * @brief Clean up the label's allocated Pango resources
 */
void Label::releaseResources() {
    // release font descriptor
    if(this->fontDesc) {
        pango_font_description_free(this->fontDesc);
        this->fontDesc = nullptr;
    }

    // release text layout context
    if(this->layout) {
        g_object_unref(this->layout);
        this->layout = nullptr;
    }

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
    int width, height;
    const auto &bounds = this->getBounds();

    // initialize layout if needed
    if(!this->layout) {
        this->layout = pango_cairo_create_layout(drawCtx);
    }

    this->updateLayout();

    // shape text and get its size
    pango_layout_set_width(this->layout, bounds.size.width * PANGO_SCALE);
    pango_layout_set_height(this->layout, bounds.size.height * PANGO_SCALE);

    pango_cairo_update_layout(drawCtx, this->layout);
    pango_layout_get_size(this->layout, &width, &height);

    // render it
    cairo::SetSource(drawCtx, this->foreground);
    pango_cairo_show_layout(drawCtx, this->layout);

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
        switch(this->align) {
            case TextAlign::Left:
                pango_layout_set_alignment(this->layout, PANGO_ALIGN_LEFT);
                break;
            case TextAlign::Center:
                pango_layout_set_alignment(this->layout, PANGO_ALIGN_CENTER);
                break;
            case TextAlign::Right:
                pango_layout_set_alignment(this->layout, PANGO_ALIGN_RIGHT);
                break;
        }

        pango_layout_set_justify(this->layout, this->justified);

        this->alignDirty = false;
    }

    // word wrapping state (either word or char boundaries)
    if(this->wordWrapDirty) {
        if(this->wordWrap) {
            pango_layout_set_wrap(this->layout, PANGO_WRAP_WORD);
        } else {
            pango_layout_set_wrap(this->layout, PANGO_WRAP_CHAR);
        }
        this->wordWrapDirty = false;
    }

    // ellipsization mode
    if(this->ellipsizationDirty) {
        switch(this->ellipsizationMode) {
            case EllipsizeMode::None:
                pango_layout_set_ellipsize(this->layout, PANGO_ELLIPSIZE_NONE);
                break;
            case EllipsizeMode::Start:
                pango_layout_set_ellipsize(this->layout, PANGO_ELLIPSIZE_START);
                break;
            case EllipsizeMode::Middle:
                pango_layout_set_ellipsize(this->layout, PANGO_ELLIPSIZE_MIDDLE);
                break;
            case EllipsizeMode::End:
                pango_layout_set_ellipsize(this->layout, PANGO_ELLIPSIZE_END);
                break;
        }

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

    this->fontDesc = pango_font_description_from_string(name.data());
    pango_font_description_set_size(this->fontDesc, size * PANGO_SCALE);

    this->fontDirty = true;
}

