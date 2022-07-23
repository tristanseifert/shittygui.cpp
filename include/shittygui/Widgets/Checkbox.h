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
