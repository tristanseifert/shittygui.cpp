#ifndef SHITTYGUI_TEXTRENDERING_H
#define SHITTYGUI_TEXTRENDERING_H

#include <string_view>

namespace shittygui {
/**
 * @brief Text rendering helper class
 *
 * This is a helper class that provides a small wrapper around Pango and the Cairo rendering
 * integration to allow widgets to render text strings. It manages the lifecycle of the underlying
 * layout object, and provides the class some methods rendering text.
 */
class TextRendering {
    public:
        ~TextRendering();

    protected:
        /// Vertical text alignment
        enum class VerticalAlign {
            Top,
            Middle,
            Bottom
        };

        /// Check whether we have text resources instantiated
        constexpr inline bool hasTextResources() const {
            return (this->layout != nullptr);
        }
        void initTextResources(struct _cairo *drawCtx);
        void releaseTextResources();

        struct _PangoFontDescription *getFont(const std::string_view name, const double size) const;
        void drawString(struct _cairo *drawCtx, const Rect &bounds, const Color &color,
                const std::string_view &str, const VerticalAlign valign = VerticalAlign::Top);
        void drawString(struct _cairo *drawCtx, const Rect &bounds, const Color &color,
                const VerticalAlign valign = VerticalAlign::Top);

        void setTextLayoutAlign(const TextAlign newAlign, const bool justified);
        void setTextLayoutEllipsization(const EllipsizeMode newMode);
        void setTextLayoutWrapMode(const bool multiParagraph, const bool wordWrap);

    protected:
        /// Pango text layout object
        struct _PangoLayout *layout{nullptr};
};
}

#endif
