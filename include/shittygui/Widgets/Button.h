#ifndef SHITTYGUI_WIDGETS_BUTTON_H
#define SHITTYGUI_WIDGETS_BUTTON_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

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
        enum class Type {
            Push,
            Square,
            Help,
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

    private:
        void releaseResources();

        void updateTextLayout();

        void drawPushButton(struct _cairo *drawCtx, const bool everything);
        void drawHelpButton(struct _cairo *drawCtx, const bool everything);

    private:
        /// Default button font
        constexpr static const std::string_view kDefaultFont{"Liberation Sans Bold"};
        /// Default button font size
        constexpr static const double kDefaultFontSize{18.};

        /// Button type
        Type type{Type::Push};

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

        /// String displayed inside a push button
        std::string title;
        /// Font to render title with
        struct _PangoFontDescription *fontDesc{nullptr};

        /// Set when the title changes
        uintptr_t titleDirty            :1{false};
        /// Set when the font has been changed
        uintptr_t fontDirty             :1{false};

        /// Is the button active/selected?
        uintptr_t selected              :1{false};
};
}

#endif
