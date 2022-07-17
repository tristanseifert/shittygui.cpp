#ifndef SHITTYGUI_WIDGETS_IMAGEVIEW_H
#define SHITTYGUI_WIDGETS_IMAGEVIEW_H

#include <cstddef>
#include <memory>
#include <utility>

#include <shittygui/Image.h>
#include <shittygui/Widget.h>
#include <shittygui/Types.h>

namespace shittygui::widgets {
/**
 * @brief Static image veiw
 *
 * Renders a static image. The image may optionally receive a border. You can also configure how
 * the image is to be scaled (not at all, proportionally up and/or down, or each axis independently).
 */
class ImageView: public Widget {
    public:
        /**
         * @brief Image rendering modes
         */
        enum class Mode {
            /**
             * @brief Render the image as-is.
             */
            None,
            /**
             * @brief Proportional scaling, shrink only
             */
            ScaleProportionalDown,
            /**
             * @brief Proportional scaling, embiggen and shrink
             */
            ScaleProportionalUpDown,
            /**
             * @brief Both axes are scaled independently
             */
            ScaleIndependently,
        };

        /**
         * @brief Create an image view without an image
         *
         * @param rect Frame rectangle for the widget
         */
        ImageView(const Rect &rect) : Widget(rect) {}

        /**
         * @brief Create an image view with an existing image
         *
         * @param rect Frame rectangle for the widget
         * @param image Image to show
         * @param mode Image display mode
         */
        ImageView(const Rect &rect, const std::shared_ptr<Image> &image,
                const Mode mode = Mode::ScaleProportionalDown) : Widget(rect), imageMode(mode) {
            this->setImage(image);
        }

        void draw(struct _cairo *drawCtx, const bool everything) override;

        /**
         * @brief Dirty the image transform matrix when our frame (size) changes
         */
        void frameDidChange() override {
            this->imageMatrixDirty = true;
        }

        /**
         * @brief Get if the image view is opaque
         *
         * This is defined by the background color, which shows through under transparent images.
         * To layer a transparent image directly, set the background color to clear.
         */
        bool isOpaque() override {
            return this->backgroundColor.isOpaque();
        }

        /**
         * @brief Set the image to be displayed
         *
         * @param newImage Image to render in the widget; a strong reference is taken.
         */
        inline void setImage(const std::shared_ptr<Image> &newImage) {
            this->image = newImage;
            this->imageMatrixDirty = true;
            this->needsDisplay();
        }
        /**
         * @brief Get the currently displayed image
         */
        constexpr inline auto &getImage() const {
            return this->image;
        }

        /**
         * @brief Set the background color
         *
         * @param newColor New background color
         */
        inline void setBackgroundColor(const Color &newColor) {
            this->backgroundColor = newColor;
            this->needsDisplay();
        }
        /**
         * @brief Get the current background color
         */
        constexpr inline auto getBackgroundColor() const {
            return this->backgroundColor;
        }

        /**
         * @brief Set the width of the border
         *
         * @param newWidth New border width; set to 0 to disable.
         */
        inline void setBorderWidth(const double newWidth) {
            this->borderWidth = std::max(0., newWidth);
            this->imageMatrixDirty = true;
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
        void drawImage(struct _cairo *, const Rect &);
        void updateImageTransform(const Rect &);

        /**
         * @brief Proportionally scale an image
         *
         * @param srcSize Input image size
         * @param maxSize Available space for the image
         * @param outSize Output size of image (with fractional components)
         */
        constexpr static inline void GetProportionalFit(const Size srcSize, const Size maxSize,
                std::pair<double, double> &outSize) {
            const auto ratio = std::min(
                    static_cast<double>(maxSize.width) / static_cast<double>(srcSize.width),
                    static_cast<double>(maxSize.height) / static_cast<double>(srcSize.height));

            outSize = {static_cast<double>(srcSize.width) * ratio,
                static_cast<double>(srcSize.height) * ratio};
        }

    private:
        /// Set to draw an outline around the image rect (for debugging)
        constexpr static bool kDrawImageOutline{false};

        /// Image to render in the view
        std::shared_ptr<Image> image;
        /// Rect to draw the image in
        Rect imageRect;
        /// Horizontal scale factor for the image
        double imageXScale{1.};
        /// Vertical scale factor for the image
        double imageYScale{1.};
        /// Image rendering mode
        Mode imageMode{Mode::None};

        /// Border thickness
        double borderWidth{1.};
        /// Color of the image border
        Color borderColor{.33, .33, .33};

        /// Background color (behind transparent images)
        Color backgroundColor{0, 0, 0};

        /// Is the cached image rendering information dirty?
        uintptr_t imageMatrixDirty              :1{true};
};
}

#endif
