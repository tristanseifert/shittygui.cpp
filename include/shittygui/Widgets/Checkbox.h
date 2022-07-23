#ifndef SHITTYGUI_WIDGETS_CHECKBOX_H
#define SHITTYGUI_WIDGETS_CHECKBOX_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include <shittygui/Widgets/ToggleButtonBase.h>

namespace shittygui::widgets {
/**
 * @brief Binary checkbox widget
 *
 * This renders a rectangular checkbox, which is either checked or not checked. It behaves
 * similarly to a button, but automatically changes state when tapped.
 *
 * @remark This does not draw a text label; it draws only the rectangular checkbox.
 */
class Checkbox: public ToggleButtonBase {
    public:
        Checkbox(const Rect &rect) : ToggleButtonBase(rect) {}
        Checkbox(const Rect &rect, const bool isChecked) : ToggleButtonBase(rect) {
            this->setChecked(isChecked);
        }
        Checkbox(const Rect &rect, const std::string_view &label) : ToggleButtonBase(rect) {
            this->setLabel(label);
        }
        Checkbox(const Rect &rect, const bool isChecked, const std::string_view &label) :
            ToggleButtonBase(rect) {
            this->setLabel(label);
            this->setChecked(isChecked);
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
        void drawCheck(struct _cairo *, const bool) override;

    private:
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
};
}

#endif
