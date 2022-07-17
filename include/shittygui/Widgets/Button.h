#ifndef SHITTYGUI_WIDGETS_BUTTON_H
#define SHITTYGUI_WIDGETS_BUTTON_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include <shittygui/Image.h>
#include <shittygui/Widget.h>
#include <shittygui/Types.h>
#include <shittygui/TextRendering.h>

namespace shittygui::widgets {
/**
 * @brief Push button
 *
 * Push buttons can be in the regular activity style (with rounded corners,) a square style, or
 * a round "help" button.
 */
class Button: public Widget, protected TextRendering {
    public:
        /// Button rendering style
        enum class Type: uint8_t {
            /// Rounded push action button
            Push,
            /// Square push action button
            Square,
            /// Circular button with a "?" glyph inside
            Help,
        };

        /// Icon gravity (alignment)
        enum class IconGravity: uint8_t {
            Left,
            Center,
            Right,
        };

        /**
         * @brief Create a button of the specified type
         *
         * @param rect Frame rectangle
         * @param type Button type
         */
        Button(const Rect &rect, const Type type) : Widget(rect), type(type) {}
        /**
         * @brief Create a button of the specified type with a title
         *
         * @param rect Frame rectangle
         * @param type Button type
         * @param title Title for the button
         */
        Button(const Rect &rect, const Type type, const std::string_view &title) : Widget(rect),
            type(type) {
            this->setTitle(title);
        }
        ~Button();

        /**
         * @brief Draw the button
         *
         * This calls the appropriate method based on the button type.
         */
        void draw(struct _cairo *drawCtx, const bool everything) override {
            switch(this->type) {
                case Type::Push:
                    this->drawPushButton(drawCtx, everything);
                    break;

                default:
                    break;
            }

            Widget::draw(drawCtx, everything);
        }

        /**
         * @brief Release rendering resources when removed from view hierarchy
         *
         * Since the drawing context for Pango keeps a reference to the underlying Cairo context,
         * and we might get added to a different screen (with a different drawing context) this
         * is required.
         */
        void didMoveToParent() override {
            Widget::didMoveToParent();
            this->releaseTextResources();
        }

        /**
         * @brief Set the text displayed on the button
         */
        inline void setTitle(const std::string_view newTitle) {
            this->title = newTitle;
            this->titleDirty = true;
            this->needsDisplay();
        }
        /**
         * @brief Get the currently displayed button title
         */
        constexpr inline const std::string_view getTitle() const {
            return this->title;
        }

        void setFont(const std::string_view name, const double size);

        /**
         * @brief Set the text color
         *
         * The text color is used to render the title in the button.
         *
         * @param normal Text color for the normal state
         * @param selected Text color for the selected state
         */
        inline void setTextColor(const Color &normal, const Color &selected) {
            this->textColor = normal;
            this->selectedTextColor = selected;
            this->needsDisplay();
        }

        /**
         * @brief Set the icon
         *
         * The icon is by default displayed centered in the button.
         */
        inline void setIcon(const std::shared_ptr<Image> &icon) {
            this->icon = icon;
            this->iconDirty = true;
            this->needsDisplay();
        }

        /**
         * @brief Set the icon gravity
         *
         * Defines where the icon is displayed in relation to the button. In addition, certain
         * values can affect the layout of the text in relation to the icon.
         */
        inline void setIconGravity(const IconGravity newGrav) {
            this->ig = newGrav;
            this->iconGravityDirty = true;
            this->needsDisplay();
        }
        /**
         * @brief Get the currently set icon gravity
         */
        constexpr inline auto getIconGravity() const {
            return this->ig;
        }

        /**
         * @brief Set the width of the border
         *
         * @param newWidth New border width; set to 0 to disable.
         */
        inline void setBorderWidth(const double newWidth) {
            this->borderWidth = std::max(0., newWidth);
            this->needsDisplay();
        }
        /**
         * @brief Get the width of the border
         */
        constexpr inline auto getBorderWidth() const {
            return this->borderWidth;
        }

        /**
         * @brief Set the color of the border
         *
         * @param normalColor new border color
         */
        inline void setBorderColor(const Color &normalColor) {
            this->borderColor = normalColor;
            this->needsDisplay();
        }
        /**
         * @brief Get the current border color
         */
        constexpr inline auto &getBorderColor() const {
            return this->borderColor;
        }

    private:
        void releaseResources();

        void updateTextLayout();

        void drawPushButton(struct _cairo *drawCtx, const bool everything);
        void drawHelpButton(struct _cairo *drawCtx, const bool everything);

        void drawTitle(struct _cairo *drawCtx, const Rect &contentRect);
        void drawIcon(struct _cairo *drawCtx, const Rect &contentRect);

    private:
        /// Default button font
        constexpr static const std::string_view kDefaultFont{"Liberation Sans Bold"};
        /// Default button font size
        constexpr static const double kDefaultFontSize{18.};

        /// Button type
        Type type{Type::Push};
        /// Icon gravity
        IconGravity ig{IconGravity::Center};

        /// Border color
        Color borderColor{.5, .5, .5};
        /// Border width
        double borderWidth{1.};
        /// Border radius (for standard push buttons)
        double borderRadius{3.};

        /// Text color (normal state)
        Color textColor{.92, .92, .92};
        /// Filling color (normal state)
        Color fillingColor{.125, .125, .125};
        /// Text color (selected state)
        Color selectedTextColor{1, 1, 1};
        /// Filling color (selected state)
        Color selectedFillingColor{.42, .42, .42};

        /// Help button content color
        Color helpContentColor{161./255., 69./255., 252./255.};

        /// Icon displayed on the push button
        std::shared_ptr<Image> icon;
        /// Padding between icon and edge of content area
        uint16_t iconPadding{2};
        /// Rect into which the icon was drawn
        Rect iconRect;

        /// String displayed inside a push button
        std::string title;
        /// Font to render title with
        struct _PangoFontDescription *fontDesc{nullptr};

        /// Set when the title changes
        uintptr_t titleDirty            :1{false};
        /// Set when the font has been changed
        uintptr_t fontDirty             :1{false};
        /// Set when the icon changed
        uintptr_t iconDirty             :1{false};
        /// Set when the icon gravity changed
        uintptr_t iconGravityDirty      :1{false};

        /// Shall the title be drawn?
        uintptr_t shouldRenderTitle     :1{true};
        /// Should the icon be drawn?
        uintptr_t shouldRenderIcon      :1{true};
        /// Is the button active/selected?
        uintptr_t selected              :1{false};
};
}

#endif
