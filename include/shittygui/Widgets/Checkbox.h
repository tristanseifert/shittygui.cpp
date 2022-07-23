#ifndef SHITTYGUI_WIDGETS_CHECKBOX_H
#define SHITTYGUI_WIDGETS_CHECKBOX_H

#include <cstddef>
#include <cstdint>
#include <optional>

#include <shittygui/Widget.h>
#include <shittygui/Types.h>

namespace shittygui::widgets {
/**
 * @brief Binary checkbox widget
 *
 * This renders a rectangular checkbox, which is either checked or not checked. It behaves
 * similarly to a button, but automatically changes state when tapped.
 *
 * @remark This does not draw a text label; it draws only the rectangular checkbox.
 */
class Checkbox: public Widget {
    public:
        Checkbox(const Rect &rect) : Widget(rect) {}
        Checkbox(const Rect &rect, const bool isChecked) : Widget(rect) {
            this->setChecked(isChecked);
        }

        void draw(struct _cairo *drawCtx, const bool everything) override;

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

        /**
         * @brief Set the radius of the container's border
         */
        inline void setBorderRadius(const double newRadius) {
            this->borderRadius = newRadius;
            this->needsDisplay();
        }
        /**
         * @brief Get the current border radius
         */
        constexpr inline auto getBorderRadius() const {
            return this->borderRadius;
        }

        /**
         * @brief Set the regular filling color
         */
        inline void setRegularFillingColor(const Color &color) {
            this->fillingColor = color;
        }
        /**
         * @brief Get the current regular filling color
         */
        constexpr inline auto &getRegularFillingColor() const {
            return this->fillingColor;
        }

        /**
         * @brief Set the selected filling color
         */
        inline void setSelectedFillingColor(const Color &color) {
            this->selectedFillingColor = color;
        }
        /**
         * @brief Get the current selected filling color
         */
        constexpr inline auto &getSelectedFillingColor() const {
            return this->selectedFillingColor;
        }

        /**
         * @brief Set the regular check color
         */
        inline void setRegularCheckColor(const Color &color) {
            this->checkColor = color;
        }
        /**
         * @brief Get the current regular check color
         */
        constexpr inline auto &getRegularCheckColor() const {
            return this->checkColor;
        }

        /**
         * @brief Set the selected check color
         */
        inline void setSelectedCheckColor(const Color &color) {
            this->selectedCheckColor = color;
        }
        /**
         * @brief Get the current selected check color
         */
        constexpr inline auto &getSelectedCheckColor() const {
            return this->selectedCheckColor;
        }

    private:
        /// Callback to invoke when button is pushed
        std::optional<EventCallback> pushCallback;

        /// Border color
        Color borderColor{.5, .5, .5};
        /// Border width
        double borderWidth{1.};
        /// Border radius (for standard push buttons)
        double borderRadius{3.};

        /// Filling color (normal state)
        Color fillingColor{.125, .125, .125};
        /// Check color (normal state)
        Color checkColor{.74, .15, .15};

        /// Filling color (selected state)
        Color selectedFillingColor{.42, .42, .42};
        /// Check color (selected state)
        Color selectedCheckColor{.74, .25, .25};

        /// Whether the checkbox is checked or not
        uintptr_t checked                       :1{false};
        /// Whether the checkbox is selected (touch down)
        uintptr_t selected                      :1{false};
};
}

#endif
