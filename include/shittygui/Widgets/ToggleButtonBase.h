#ifndef SHITTYGUI_WIDGETS_TOGGLEBUTTONBASE_H
#define SHITTYGUI_WIDGETS_TOGGLEBUTTONBASE_H
 
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include <shittygui/Widget.h>
#include <shittygui/Types.h>
#include <shittygui/TextRendering.h>

namespace shittygui::widgets {
/**
 * @brief Abstract base class for toggle buttons (checkbox, radio button)
 */
class ToggleButtonBase: public Widget, protected TextRendering {
    public:
        ToggleButtonBase(const Rect &rect) : Widget(rect) {}
        virtual ~ToggleButtonBase();

        /**
         * @brief Draw the checkbox
         */
        void draw(struct _cairo *drawCtx, const bool everything) override {
            this->drawCheck(drawCtx, everything);

            if(this->label.has_value()) {
                this->drawLabel(drawCtx, everything);
            }
        }

        /**
         * @brief Release text rendering resources when moving around.
         */
        void didMoveToParent() override {
            Widget::didMoveToParent();
            this->releaseTextResources();
        }

        /**
         * @brief Update the checked state
         *
         * @param checked Whether the checkbox should be drawn as checked or not
         */
        constexpr inline void setChecked(const bool checked) {
            this->checked = checked;
            this->needsDisplay();
        }
        /**
         * @brief Get the checked state
         *
         * @return Whether the checkbox is checked
         */
        constexpr inline bool isChecked() const {
            return this->checked;
        }

        /**
         * @brief Opt in to touch tracking
         *
         * This is used so we can update the selection state of the checkbox when touches leave
         * the checkbox.
         */
        bool wantsTouchTracking() override {
            return true;
        }

        bool handleTouchEvent(const event::Touch &event) override;

        /**
         * @brief Set the click callback
         *
         * This is executed when a touch is _up_ inside the bounds of the checkbox, resulting in
         * a value changing.
         */
        inline void setPushCallback(const EventCallback &cb) {
            this->pushCallback = cb;
        }
        /**
         * @brief Remove any existing push callback
         */
        inline void resetPushCallback() {
            this->pushCallback.reset();
        }

        /**
         * @brief Set whether touch events are recognized in the check area only
         *
         * This controls if touch events in the label area are also recognized and can change the
         * value of the checkbox.
         */
        inline void setCheckAreaTouchOnly(const bool flag) {
            this->touchInsideCheckOnly = flag;
        }
        /**
         * @brief Get whether checkboxes accept touch events only inside the check area
         */
        constexpr inline auto getCheckAreaTouchOnly() const {
            return this->touchInsideCheckOnly;
        }

        void setFont(const std::string_view name, const double size);

        /**
         * @brief Set the text color
         *
         * The text color is used to render the checkbox label.
         *
         * @param normal Text color for the normal state
         */
        inline void setTextColor(const Color &normal) {
            this->textColor = normal;
            this->needsDisplay();
        }
        /**
         * @brief Set the checkbox label
         */
        inline void setLabel(const std::string_view &label) {
            this->label = label;
            this->labelDirty = true;
            this->needsDisplay();
        }

    protected:
        /**
         * @brief Draw the base of the button
         *
         * This would render the checkbox or radio button part. It shall then update the checkRect
         * to indicate what space was used by it.
         */
        virtual void drawCheck(struct _cairo *, const bool) = 0;

        void drawLabel(struct _cairo *, const bool);
        void updateTextLayout();

        /**
         * @brief Update internal state in response to a touch event
         */
        virtual void updateStateFromTouch() = 0;

    protected:
        /// Callback to invoke when button is pushed
        std::optional<EventCallback> pushCallback;

        /// Rectangle used to draw the checkbox
        Rect checkRect;

        /// Whether the button is checked or not
        uintptr_t checked                       :1{false};
        /// Whether the button is selected (touch down)
        uintptr_t selected                      :1{false};

    private:
        /// Text content has changed
        uintptr_t labelDirty                    :1{false};
        /// Font has changed
        uintptr_t fontDirty                     :1{false};
        /// Accept events only inside the check area
        uintptr_t touchInsideCheckOnly          :1{false};

        /// Default button font
        constexpr static const std::string_view kDefaultFont{"Liberation Sans Bold"};
        /// Default button font size
        constexpr static const double kDefaultFontSize{18.};

        /// Color for the text label
        Color textColor{1., 1., 1.};
        /// Text label
        std::optional<std::string> label;
        /// Font to render title with
        struct _PangoFontDescription *fontDesc{nullptr};
};
};

#endif
