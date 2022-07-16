#ifndef SHITTYGUI_SCREEN_H
#define SHITTYGUI_SCREEN_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>

#include <shittygui/Types.h>

namespace shittygui {
class Widget;

/**
 * @brief GUI screen class
 *
 * Screens serve as the base of the GUI library. They accumulate inputs from various sources,
 * handle the lifecycle of controls, and draw controls (as needed, based on their dirty flags) into
 * their underlying framebuffer.
 */
class Screen {
    public:
        enum class PixelFormat {
            /// 24-bit color in a 32-bit value; upper 8 bits alpha, premultiplied
            ARGB32,
            /// 24-bit color in a 32-bit value; upper 8 bits unused
            RGB24,
            /// 16-bit color, split 5-6-5 between RGB
            RGB16,
            /// 30-bit color, 10 bits per color channel
            RGB30,
        };

        Screen(const PixelFormat format, const Size &size);
        Screen(const PixelFormat format, const Size &size, std::span<std::byte> framebuffer,
                const size_t stride);
        ~Screen();

        /// Get the physical dimensions of the underlying framebuffer
        inline constexpr auto getFramebufferSize() {
            return this->physSize;
        }
        /// Get the dimensions of the screen
        inline constexpr auto getSize() const {
            return this->size;
        }
        /// Get the screen's pixel format
        inline constexpr auto getPixelFormat() const {
            return this->format;
        }
        void *getBuffer();
        size_t getBufferStride() const;

        static size_t OptimalStrideForBuffer(const PixelFormat format, const uint16_t width);
        inline static size_t OptimalStrideForBuffer(const PixelFormat format, const Size &size) {
            return OptimalStrideForBuffer(format, size.width);
        }

        /**
         * @brief Set the UI scale factor
         *
         * The scale factor can be used to support high pixel density displays. Positive values
         * above will increase the size of UI components, and values below 1 (and above 0) will
         * shrink them.
         */
        inline void setScaleFactor(const double scale) {
            this->scaleFactor = scale;
            this->scaled = true;
        }

        /**
         * @brief Determine if the screen needs to be redrawn
         *
         * The screen is dirtied if any of the components in the widget hierarchy become dirty,
         * usually in response to some sort of external event.
         */
        inline bool isDirty() const {
            return this->dirtyFlag;
        }
        /// Mark the screen as needing to be redrawn
        inline void needsDisplay() {
            this->forceDisplayFlag = true;
            this->dirtyFlag = true;
        }

        /**
         * @brief Update the root widget of the screen
         *
         * Replace the existing root widget with this new widget, then invalidate the screen so
         * it's redrawn.
         *
         * @param newRoot Widget to set as the root
         */
        inline void setRootWidget(const std::shared_ptr<Widget> &newRoot) {
            this->rootWidget = newRoot;
            this->needsDisplay();
        }
        /**
         * @brief Get the current root widget
         *
         * @return Current root widget (if any has been set)
         */
        inline auto getRootWidget() {
            return this->rootWidget;
        }

        void redraw();
        void handleAnimations();

        /**
         * @brief Set the screen's background color
         *
         * The background color is visible anywhere the root view does not draw (if it's not fully
         * opaque) or if there is no root view.
         *
         * @param newColor Color to draw as the background. It should have an alpha value of 1.
         */
        inline void setBackgroundColor(const Color &newColor) {
            this->backgroundColor = newColor;
            this->needsDisplay();
        }

    private:
        void commonInit();

    private:
        /// Pixel format of the screen
        PixelFormat format;

        /// Physical size of the output framebuffer
        Size physSize;
        /// Dimensions of the rendering surface (takes into account rotation)
        Size size;
        /// User interface scale factor
        double scaleFactor{1.};

        /// Underlying Cairo rendering surface
        struct _cairo_surface *surface{nullptr};
        /// Cairo drawing context, backed by the framebuffer surface
        struct _cairo *drawCtx{nullptr};

        /// Screen background color
        Color backgroundColor;
        /// Root widget, which receives all events and draw requests
        std::shared_ptr<Widget> rootWidget;

        /// Set when any widget in this screen becomes dirty
        bool dirtyFlag{false};
        /// Set to force rendering of _all_ widgets regardless of dirty status
        bool forceDisplayFlag{false};
        /// Whether the screen applies UI scaling
        bool scaled{false};
};
}

#endif
