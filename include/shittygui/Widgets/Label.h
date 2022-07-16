#ifndef SHITTYGUI_WIDGETS_LABEL_H
#define SHITTYGUI_WIDGETS_LABEL_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include <shittygui/Widget.h>
#include <shittygui/Types.h>

namespace shittygui::widgets {
/**
 * @brief Static (read-only) text label
 *
 * Renders a read-only text string to the screen. The font, color, and alignment can be specified,
 * and the text may be optionally wrapped to fit in the available space.
 *
 */
class Label: public Widget {
    public:
        /**
         * @brief Initialize a label with the given frame
         *
         * It will not have a content string.
         */
        Label(const Rect &rect) : Widget(rect) {}
        /**
         * @brief Initialize a label with given frame and content
         */
        Label(const Rect &rect, const std::string_view text) : Widget(rect) {
            this->setContent(text);
        }

        /**
         * @brief Free all rendering resources belonging to the label.
         */
        ~Label() {
            this->releaseResources();
        }

        /**
         * Labels are not opaque; they rely on the underlying view to fill its background when we
         * are dirtied. This is taken care of by container views.
         */
        bool isOpaque() override {
            return false;
        }

        void draw(struct _cairo *drawCtx, const bool everything) override;

        /**
         * @brief Release rendering resources when removed from view hierarchy
         *
         * Since the drawing context for Pango keeps a reference to the underlying Cairo context,
         * and we might get added to a different screen (with a different drawing context) this
         * is required.
         */
        void orphaned() override {
            this->releaseResources();
        }

        /**
         * @brief Set the text displayed on the label
         */
        inline void setContent(const std::string_view newContent) {
            this->content = newContent;
            this->contentDirty = true;
            this->needsDisplay();
        }
        /**
         * @brief Get the currently displayed label text
         */
        constexpr inline const std::string_view getContent() const {
            return this->content;
        }

        /**
         * @brief Set the text alignment
         */
        inline void setTextAlign(const TextAlign newAlign) {
            this->align = newAlign;
            this->alignDirty = true;
            this->needsDisplay();
        }
        /**
         * @brief Get the current text alignment
         */
        constexpr inline auto getTextAlign() const {
            return this->align;
        }

        /**
         * @brief Set whether lines are justified
         */
        inline void setJustified(const bool isJustified) {
            this->justified = isJustified;
            this->alignDirty = true;
            this->needsDisplay();
        }
        /**
         * @brief Get whether lines are justified
         */
        constexpr inline bool isJustified() const {
            return this->justified;
        }

        /**
         * @brief Set the wrapping mode
         *
         * @param enabled Whether lines are wrapped at word boundaries
         */
        inline void setWordWrap(const bool enabled) {
            this->wordWrap = enabled;
            this->wordWrapDirty = true;
            this->needsDisplay();
        }
        /**
         * @brief Get if lines are wrapped at word boundaries
         */
        constexpr inline bool getWordWrap() const {
            return this->wordWrap;
        }

        /**
         * @brief Set the ellipsization mode
         */
        inline void setEllipsizeMode(const EllipsizeMode newMode) {
            this->ellipsizationMode = newMode;
            this->ellipsizationDirty = true;
            this->needsDisplay();
        }
        /**
         * @brief Get the current ellipsization mode
         */
        constexpr inline auto getEllipsizeMode() const {
            return this->ellipsizationMode;
        }

        /**
         * @brief Set the text color
         */
        inline void setTextColor(const Color &newColor) {
            this->foreground = newColor;
            this->needsDisplay();
        }
        /**
         * @brief Get the text color
         */
        constexpr inline auto getTextColor() const {
            return this->foreground;
        }

        void setFont(const std::string_view name, const double size);

    private:
        void releaseResources();

        void updateLayout();

    private:
        /// Text layout
        TextAlign align{TextAlign::Left};
        /// Ellipsization mode
        EllipsizeMode ellipsizationMode{EllipsizeMode::End};

        /// Text foreground color
        Color foreground;

        /// Content value of the label
        std::string content;

        /// Pango font descriptor for the label's font
        struct _PangoFontDescription *fontDesc{nullptr};
        /// Pango text layout object
        struct _PangoLayout *layout{nullptr};

        /// Set when the text content changes
        uintptr_t contentDirty          :1{false};
        /// Set when the font has been changed
        uintptr_t fontDirty             :1{false};
        /// Set when the alignment changes (includes justification)
        uintptr_t alignDirty            :1{true};
        /// Set when the word wrap state changes
        uintptr_t wordWrapDirty         :1{true};
        /// Set when the ellipsization mode changes
        uintptr_t ellipsizationDirty    :1{true};

        /// Is text justified?
        uintptr_t justified             :1{false};
        /// Does the text word wrap to support multiple lines?
        uintptr_t wordWrap              :1{false};
};
}

#endif