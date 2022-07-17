#include <cairo.h>
#include <pango/pangocairo.h>

#include "CairoHelpers.h"
#include "Util.h"
#include "TextRendering.h"

using namespace shittygui;

/**
 * @brief Release resources
 */
TextRendering::~TextRendering() {
    this->releaseTextResources();
}

/**
 * @brief Initialize the Pango text layout context
 *
 * This context is by default configured for single paragraph operation, with left alignment.
 *
 * @param drawCtx Cairo drawing context to render text onto
 */
void TextRendering::initTextResources(cairo_t *drawCtx) {
    this->layout = pango_cairo_create_layout(drawCtx);

    this->setTextLayoutWrapMode(false, true);
    this->setTextLayoutAlign(TextAlign::Left, false);
}

/**
 * @brief Clean up the allocated Pango resources
 */
void TextRendering::releaseTextResources() {
    if(this->layout) {
        g_object_unref(this->layout);
        this->layout = nullptr;
    }
}

/**
 * @brief Parse a font descriptor string
 *
 * Fonts are automatically loaded using the system's font discovery mechanism. Names are parsed as
 * [Pango FontDescriptions](https://docs.gtk.org/Pango/type_func.FontDescription.from_string.html)
 * so you can customize the style, variants, weight, gravity, and stretch values of the font.
 *
 * @param name Font name
 * @param size Font size, in points
 */
PangoFontDescription *TextRendering::getFont(const std::string_view name, const double size) const {
    auto desc = pango_font_description_from_string(name.data());
    pango_font_description_set_size(desc, size * PANGO_SCALE);

    return desc;
}

/**
 * @brief Render a specified string set on the layout context
 *
 * @param Cairo drawing context to render into
 * @param bounds Frame rectangle of the resulting text
 * @param color Color to render the text in
 * @param str String to render (it is applied to the layout)
 *
 * @remark The render context should have its "current point" set as the origin of the text.
 */
void TextRendering::drawString(cairo_t *drawCtx, const Rect &bounds, const Color &color,
        const std::string_view &str) {
    pango_layout_set_text(this->layout, str.data(), str.length());
    this->drawString(drawCtx, bounds, color);
}

/**
 * @brief Render the last string set on the layout context
 *
 * @param Cairo drawing context to render into
 * @param bounds Frame rectangle of the resulting text
 * @param color Color to render the text in
 *
 * @remark The render context should have its "current point" set as the origin of the text.
 */
void TextRendering::drawString(cairo_t *drawCtx, const Rect &bounds, const Color &color) {
    pango_layout_set_width(this->layout, bounds.size.width * PANGO_SCALE);
    pango_layout_set_height(this->layout, bounds.size.height * PANGO_SCALE);

    pango_cairo_update_layout(drawCtx, this->layout);

    cairo::SetSource(drawCtx, color);
    pango_cairo_show_layout(drawCtx, this->layout);
}

/**
 * @brief Update the text alignment and justification settings of the text layout context
 *
 * @param newAlign New horizontal alignment setting
 * @param justified Whether text is justified
 */
void TextRendering::setTextLayoutAlign(const TextAlign newAlign, const bool justified) {
    switch(newAlign) {
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

    pango_layout_set_justify(this->layout, justified);
}

/**
 * @brief Update the text ellipsization mode of the text drawing context
 *
 * @param newMode Text ellipsization mode to set
 */
void TextRendering::setTextLayoutEllipsization(const EllipsizeMode newMode) {
    switch(newMode) {
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
}

/**
 * @brief Update the wrapping and line break mode
 *
 * @param multiParagraph Whether the text renderer renders multiple paragraphs
 * @param wordWrap Whether lines are wrapped on word (`true`) or character (`false`) boundaries
 */
void TextRendering::setTextLayoutWrapMode(const bool multiParagraph, const bool wordWrap) {
    if(wordWrap) {
        pango_layout_set_wrap(this->layout, PANGO_WRAP_WORD);
    } else {
        pango_layout_set_wrap(this->layout, PANGO_WRAP_CHAR);
    }
    pango_layout_set_single_paragraph_mode(this->layout, !multiParagraph);
}
