#ifndef SHITTYGUI_WIDGETS_CONTAINER_H
#define SHITTYGUI_WIDGETS_CONTAINER_H

#include <shittygui/Widget.h>
#include <shittygui/Types.h>

namespace shittygui::widgets {
/**
 * @brief Generic object container
 *
 * This is a very basic widget whose only job it is to hold other widgets. It can be decorated with
 * an optional border, and draws an opaque background inside its bounds.
 */
class Container: public Widget {
    public:
        /**
         * @brief Initialize a new container widget
         */
        Container(const Rect &rect) : Widget(rect) {}

        void draw(struct _cairo *drawCtx, const bool everything) override;

        bool isOpaque() override {
            return this->background.isOpaque();
        }

        /**
         * @brief Set the background color of the container
         */
        inline void setBackgroundColor(const Color &newColor) {
            this->background = newColor;
            this->needsDisplay();
        }
        /**
         * @brief Get the current background color
         */
        inline auto &getBackgroundColor() const {
            return this->background;
        }

        /**
         * @brief Set the border color of the container
         */
        inline void setBorderColor(const Color &newColor) {
            this->border = newColor;
            this->needsDisplay();
        }
        /**
         * @brief Get the current border color
         */
        inline auto &getBorderColor() const {
            return this->border;
        }

        /**
         * @brief Change whether the container draws a border
         */
        inline void setDrawsBorder(const bool draw) {
            this->drawBorder = draw;
            this->needsDisplay();
        }
        /**
         * @brief Get whether the container draws a border
         */
        inline auto getDrawsBorder() const {
            return this->drawBorder;
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
        inline auto getBorderRadius() const {
            return this->borderRadius;
        }

    private:
        /// Width of the border, in pixels
        constexpr static const double kBorderWidth{1.};

        /// Background color
        Color background;
        /// Border color
        Color border{0, 1, 0};
        /// Border corner radius
        double borderRadius{5.};

        /// Whether the border shall be drawn
        bool drawBorder{true};
};
}

#endif
