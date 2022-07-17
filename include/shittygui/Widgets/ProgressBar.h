#ifndef SHITTYGUI_WIDGETS_PROGRESSBAR_H
#define SHITTYGUI_WIDGETS_PROGRESSBAR_H

#include <cstdint>

#include <shittygui/Widget.h>
#include <shittygui/Types.h>

namespace shittygui::widgets {
/**
 * @brief Horizontal progress bar
 *
 * This widget draws a rectangular, horizontal bar which can be used to indicate the progress of
 * an action. The bar can either have a determinate style (with a progress percentage, which
 * determines how much of the bar is filled) or an indeterminate style, where the entire bar is
 * instead replaced with an animated barber pole pattern.
 */
class ProgressBar: public Widget {
    public:
        /**
         * @brief Appearance style of a progress bar
         */
        enum class Style: uint8_t {
            /**
             * @brief Determinate style
             *
             * The standard progress bar style, where part of the bar is filled, corresponding to
             * a fraction percentage.
             */
            Determinate,
            /**
             * @brief Indeterminate style
             *
             * An indeterminate progress bar is used to indicate that work is taking place, but
             * no useful measure of its progress is available. It's rendered as an animated barber
             * pole pattern.
             */
            Indeterminate,
        };

        /**
         * @brief Initialize a progress bar with the given frame
         *
         * The bar will be the determinate style.
         */
        ProgressBar(const Rect &rect) : Widget(rect) {}

        /**
         * @brief Initialize a progress bar with the given frame and style
         */
        ProgressBar(const Rect &rect, const Style style) : Widget(rect), style(style) {}

        /**
         * @brief Clean up all our internal resources
         */
        ~ProgressBar() {
            this->releaseResources();
        }

        void draw(struct _cairo *drawCtx, const bool everything) override;

        /**
         * @brief Opt in to animation support
         */
        inline bool wantsAnimation() override {
            return true;
        }
        void processAnimationFrame() override;

        /**
         * @brief Release resources when removed from view hierarchy
         */
        void didMoveToParent() override {
            Widget::didMoveToParent();
            this->releaseResources();
        }
        /**
         * @brief When the frame changes, re-create the fill patterns
         */
        void frameDidChange() override {
            this->fillDirty = true;
            this->needsDisplay();
        }

        /**
         * @brief Set the current progress value
         *
         * @param newProgress Progress percentage {0, 1}
         */
        inline void setProgress(const double newProgress) {
            this->progress = std::max(std::min(newProgress, 1.), 0.);
            this->needsDisplay();
        }
        /**
         * @brief Get the current progress percentage
         */
        constexpr inline auto getProgress() const {
            return this->progress;
        }

        /**
         * @brief Set the bar style
         */
        inline void setStyle(const Style newStyle) {
            this->style = newStyle;
        }
        /**
         * @brief Get the current progress bar style
         */
        constexpr inline auto getStyle() const {
            return this->style;
        }

    private:
        void releaseResources();

        void updateIndeterminateFill(const Rect &);
        void drawIndeterminatePattern(_cairo *, const double, const double);

    private:
        /// Bar border color
        constexpr static const Color kBorderColor{.33, .33, .33};
        /// Border width
        constexpr static const double kBorderWidth{1.};

        /// Time for one revolution in indeterminate mode (seconds)
        constexpr static const double kIndeterminateAnimInterval{.42};

        /// Current bar style
        Style style{Style::Determinate};
        /// Progress percentage (in [0, 1])
        double progress{0.};

        /// A surface containing the barber pole pattern for indeterminate bars
        struct _cairo_surface *barberSurface{nullptr};
        /// Pattern for drawing the barber pole pattern
        struct _cairo_pattern *barberPattern{nullptr};
        /// Width of the pattern (in pixels)
        double patternWidth;

        /**
         * @brief Recompute the fill pattern
         */
        uintptr_t fillDirty                     :1{false};
};
}

#endif
