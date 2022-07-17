#ifndef SHITTYGUI_SCREEN_H
#define SHITTYGUI_SCREEN_H

#include <cstddef>
#include <cstdint>
#include <deque>
#include <memory>
#include <mutex>
#include <span>

#include <shittygui/Event.h>
#include <shittygui/Types.h>

namespace shittygui {
class Animator;
class Widget;

/**
 * @brief GUI screen class
 *
 * Screens serve as the base of the GUI library. They accumulate inputs from various sources,
 * handle the lifecycle of controls, and draw controls (as needed, based on their dirty flags) into
 * their underlying framebuffer.
 */
class Screen: public std::enable_shared_from_this<Screen> {
    friend class Widget;

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

        /**
         * @brief Rotation of the logical framebuffer
         *
         * This enum defines various cases of rotation (in 90° increments) for the logical display
         * screen on the physical framebuffer. (Such a feature is useful for displays that are
         * scanned out in one orientation, but are mounted at another: for example, a portrait
         * smartphone display mounted in landscape orientation.)
         */
        enum class Rotation {
            /// No rotation, the physical framebuffer and logical framebuffer are the same
            None,
            Rotate90,
            Rotate180,
            Rotate270,
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
            this->needsDisplay();
        }

        /**
         * @brief Set the logiccal framebuffer rotation
         *
         * The logical rotation is converted to a translation matrix that appropriately rotates the
         * drawing context during rendering.
         */
        inline void setRotation(const Rotation newRotation) {
            this->rotation = newRotation;

            if(this->rotation == Rotation::Rotate90 || this->rotation == Rotation::Rotate270) {
                this->size = {this->physSize.height, this->physSize.width};
            } else {
                this->size = this->physSize;
            }

            this->needsDisplay();
        }
        /**
         * @brief Get the current display rotation
         */
        constexpr inline auto getRotation() const {
            return this->rotation;
        }

        bool isDirty() const;
        /// Mark the screen as needing to be redrawn
        inline void needsDisplay() {
            this->forceDisplayFlag = true;
            this->dirtyFlag = true;
        }

        void setRootWidget(const std::shared_ptr<Widget> &newRoot);
        /**
         * @brief Get the current root widget
         *
         * @return Current root widget (if any has been set)
         */
        inline auto getRootWidget() {
            return this->rootWidget;
        }

        /**
         * @brief Get the animator instance
         */
        inline auto &getAnimator() {
            return this->anim;
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

    public:
        void processEvents();

        /**
         * @brief Insert an event into the event queue
         *
         * @param eevnt Event to insert into the event queue
         * @param atEnd Whether the event is to go at the end or front of the event queue
         */
        inline void queueEvent(const Event event, const bool atEnd = true) {
            std::lock_guard lg(this->eventQueueLock);

            if(atEnd) {
                this->eventQueue.emplace_back(event);
            } else {
                this->eventQueue.emplace_front(event);
            }
        }

        /**
         * @brief Set a widget as the first responder
         *
         * First responder widgets will receive all modal (that is, keyboard and scroll events)
         * user input.
         */
        inline void setFirstResponder(const std::shared_ptr<Widget> &widget) {
            this->firstResponder = widget;
            this->firstResponderDirty = true;
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
        /// Display rotation
        Rotation rotation{Rotation::None};

        /// Underlying Cairo rendering surface
        struct _cairo_surface *surface{nullptr};
        /// Cairo drawing context, backed by the framebuffer surface
        struct _cairo *drawCtx{nullptr};

        /// Screen background color
        Color backgroundColor;
        /// Root widget, which receives all events and draw requests
        std::shared_ptr<Widget> rootWidget;

        /// Animation coordinator instance
        std::shared_ptr<Animator> anim;

        /// Event queue
        std::deque<Event> eventQueue;
        /// Lock protecting the event queue
        std::mutex eventQueueLock;

        /// Which widget has the current input focus
        std::weak_ptr<Widget> firstResponder;

        /// Set when any widget in this screen becomes dirty
        uintptr_t dirtyFlag                     :1{false};
        /// Set to force rendering of _all_ widgets regardless of dirty status
        uintptr_t forceDisplayFlag              :1{false};
        /// Whether the screen applies UI scaling
        uintptr_t scaled                        :1{false};
        /// The first responder widget has changed
        uintptr_t firstResponderDirty           :1{false};
};
}

#endif
