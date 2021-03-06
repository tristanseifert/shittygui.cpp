#ifndef SHITTYGUI_WIDGETS_RADIOBUTTON_H
#define SHITTYGUI_WIDGETS_RADIOBUTTON_H

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

#include <shittygui/Types.h>
#include <shittygui/Widgets/ToggleButtonBase.h>

namespace shittygui::widgets {
/**
 * @brief Binary radio button widget
 *
 * This renders a circular radio button, which is either checked or not checked. It behaves
 * similarly to a button, but automatically changes state when tapped.
 */
class RadioButton: public ToggleButtonBase {
    public:
        /**
         * @brief Information for a single radio button in a radio group
         */
        struct GroupEntry {
            /// Frame rectangle of the radio button (relative to the radio group's origin)
            Rect rect;
            /// Label to apply to the button
            std::string_view label;

            /// Tag value associated with the entry
            uintptr_t tag;

            /// Whether this option is pre-selected
            bool isChecked{false};
        };

        /**
         * @brief Callback to invoke when a radio group's value changes
         */
        using GroupCallback = std::function<void(const std::shared_ptr<RadioButton> &whomst,
                const uintptr_t tag)>;
        /**
         * @brief Callback invoked to prepare radio group members for appearance
         *
         * This can be used by applications to apply their custom styles to radio group members.
         */
        using GroupPrepareCallback = std::function<void(const std::shared_ptr<RadioButton> &)>;

        static std::shared_ptr<Widget> MakeRadioGroup(std::span<const GroupEntry> options,
                const GroupCallback &changeCb,
                std::optional<const GroupPrepareCallback> preparer = std::nullopt);

    private:
        static void ValidateAndProcessOptions(std::span<const GroupEntry> options, Size &outSize);
        static void UncheckAllOthers(const std::shared_ptr<RadioButton> &);

    public:
        RadioButton(const Rect &rect) : ToggleButtonBase(rect) {}
        RadioButton(const Rect &rect, const bool isChecked) : ToggleButtonBase(rect) {
            this->setChecked(isChecked);
        }
        RadioButton(const Rect &rect, const std::string_view &label) : ToggleButtonBase(rect) {
            this->setLabel(label);
        }
        RadioButton(const Rect &rect, const bool isChecked, const std::string_view &label) :
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
         * @brief Set the regular dot color
         */
        inline void setRegularIndicatorColor(const Color &color) {
            this->indicatorColor = color;
        }
        /**
         * @brief Get the current regular indicator color
         */
        constexpr inline auto &getRegularIndicatorColor() const {
            return this->indicatorColor;
        }

        /**
         * @brief Set the selected indicator color
         */
        inline void setSelectedIndicatorColor(const Color &color) {
            this->selectedIndicatorColor = color;
        }
        /**
         * @brief Get the current selected indicator color
         */
        constexpr inline auto &getSelectedIndicatorColor() const {
            return this->selectedIndicatorColor;
        }

    private:
        void drawCheck(struct _cairo *, const bool) override;

        /**
         * @brief Set this radio button as checked on touch event
         */
        void updateStateFromTouch() override {
            this->checked = true;
        }

    private:
        /// Border color
        Color borderColor{.5, .5, .5};
        /// Border width
        double borderWidth{1.};

        /// Filling color (normal state)
        Color fillingColor{.125, .125, .125};
        /// Indicator color (normal state)
        Color indicatorColor{.74, .15, .15};

        /// Filling color (selected state)
        Color selectedFillingColor{.42, .42, .42};
        /// Indicator color (selected state)
        Color selectedIndicatorColor{.74, .25, .25};
};

}


#endif
