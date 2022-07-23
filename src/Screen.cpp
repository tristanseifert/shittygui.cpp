#include <numeric>
#include <stdexcept>
#include <type_traits>

#include <cairo.h>

#include "Animator.h"
#include "CairoHelpers.h"
#include "Errors.h"
#include "Event.h"
#include "Screen.h"
#include "Util.h"
#include "Widget.h"
#include "ViewController.h"

using namespace shittygui;

/**
 * @brief Convert a screen pixel format to the corresponding Cairo type
 */
static inline constexpr cairo_format_t ConvertPixelFormat(const Screen::PixelFormat in) {
    switch(in) {
        case Screen::PixelFormat::ARGB32:
            return CAIRO_FORMAT_ARGB32;
        case Screen::PixelFormat::RGB24:
            return CAIRO_FORMAT_RGB24;
        case Screen::PixelFormat::RGB16:
            return CAIRO_FORMAT_RGB16_565;
        case Screen::PixelFormat::RGB30:
            return CAIRO_FORMAT_RGB30;
    }
}

/**
 * @brief Calculate the optimal stride (bytes per line) for a framebuffer of the given format+size
 *
 * @param format Pixel format of the framebuffer
 * @param width Width of the framebuffer (pixels)
 *
 * @return Most optimal stride value
 *
 * @throw std::invalid_argument Illegal pixel format or width was specified
 */
size_t Screen::OptimalStrideForBuffer(const PixelFormat format, const uint16_t width) {
    auto ret = cairo_format_stride_for_width(ConvertPixelFormat(format), width);
    if(ret == -1) {
        throw std::invalid_argument("cairo_format_stride_for_width failed");
    }

    return ret;
}



/**
 * @brief Initialize a screen with an internal framebuffer
 *
 * Create a new Screen instance, allocating internally a framebuffer of the specified format and
 * physical dimensions. The underlying framebuffer's color components will be zeroed.
 *
 * @param format Desired pixel format
 * @param size Physical size of the framebuffer, in pixels
 */
Screen::Screen(const PixelFormat format, const Size &size) : format(format), physSize(size) {
    this->surface = cairo_image_surface_create(ConvertPixelFormat(format), size.width,
            size.height);
    auto status = cairo_surface_status(this->surface);

    if(status != CAIRO_STATUS_SUCCESS) {
        ThrowForCairoStatus(status);
    }

    this->commonInit();
}

/**
 * @brief Initialize a screen with an external framebuffer
 *
 * Create a new Screen instance that renders to an externally allocated framebuffer, with the
 * specified pixel format, size, and stride.
 *
 * @param format Pixel format of the framebuffer
 * @param size Physical size of the framebuffer, in pixels
 * @param framebuffer Memory region allocated to the framebuffer
 * @param stride Bytes per row
 *
 * @throw std::runtime_error Illegal framebuffer configuration specified
 */
Screen::Screen(const PixelFormat format, const Size &size, std::span<std::byte> framebuffer,
        const size_t stride) : format(format), physSize(size) {
    this->surface = cairo_image_surface_create_for_data(
            reinterpret_cast<unsigned char *>(framebuffer.data()), ConvertPixelFormat(format),
            size.width, size.height, stride);
    auto status = cairo_surface_status(this->surface);

    if(status != CAIRO_STATUS_SUCCESS) {
        ThrowForCairoStatus(status);
    }

    this->commonInit();
}

/**
 * @brief Perform common initialization
 *
 * This sets up the Cairo drawing context.
 */
void Screen::commonInit() {
    this->drawCtx = cairo_create(this->surface);
    auto status = cairo_status(this->drawCtx);

    if(status != CAIRO_STATUS_SUCCESS) {
        ThrowForCairoStatus(status);
    }

    // optimize rendering for performance
    cairo_set_antialias(this->drawCtx, CAIRO_ANTIALIAS_FAST);

    // prepare animation resources
    this->anim = std::make_shared<Animator>(this);
}

/**
 * @brief Release all resources associated with the screen
 *
 * Release all event handling resources as well as all widgets; then release the underlying
 * drawing resources.
 */
Screen::~Screen() {
    // clear cairo resources
    cairo_destroy(this->drawCtx);
    cairo_surface_destroy(this->surface);
}

/**
 * @brief Return a pointer to the underlying framebuffer
 *
 * If the screen has an externally allocated buffer, this is the same pointer as was passed in to
 * the constructor.
 */
void *Screen::getBuffer() {
    return cairo_image_surface_get_data(this->surface);
}

/**
 * @brief Return the stride of the underlying framebuffer
 */
size_t Screen::getBufferStride() const {
    return cairo_image_surface_get_stride(this->surface);
}

/**
 * @brief Determine if the screen is dirty
 *
 * This checks our internal dirty flag, plus the dirty flag of the root view.
 */
bool Screen::isDirty() const {
    if(this->dirtyFlag || this->forceDisplayFlag) {
        return true;
    }

    if(this->rootWidget) {
        return this->rootWidget->isDirty();
    }

    return false;
}

/**
 * @brief Redraw the screen
 *
 * Draws the contents of the screen (that is, the root widget, and any descendant widgets) into the
 * underlying framebuffer. Only dirty widgets will be drawn.
 */
void Screen::redraw() {
    cairo_save(this->drawCtx);

    // apply UI scale and rotation
    if(this->scaled) {
        const double factor{this->scaleFactor};
        cairo_scale(this->drawCtx, factor, factor);
    }

    switch(this->rotation) {
        case Rotation::Rotate270:
            cairo_rotate(this->drawCtx, cairo::DegreesToRadian(270));
            cairo_translate(this->drawCtx, -this->size.width, 0);
            break;

        // nothing to be done
        case Rotation::None:
            break;

        default:
            throw std::runtime_error("unimplemented screen rotation");
    }

    // draw background if no root widget, or it's not opaque
    if(!this->rootWidget || !this->rootWidget->isOpaque()) {
        cairo::SetSource(this->drawCtx, this->backgroundColor);
        cairo_paint(this->drawCtx);
    }

    /*
     * Draw the root widget
     *
     * Note that we do not apply any clipping regions nor translate the coordinate space; this is
     * because the root widget (by definition) fills the entire screen. Requests to draw outside
     * the screen will be implicitly discarded.
     */
    if(this->rootWidget) {
        this->rootWidget->draw(this->drawCtx, this->forceDisplayFlag);
        this->rootWidget->drawChildren(this->drawCtx,
                (this->forceDisplayFlag || this->rootWidget->isDirty()));

        this->forceDisplayFlag = false;
    }

    // clear the dirty flag
    this->dirtyFlag = false;

    cairo_restore(this->drawCtx);
}

/**
 * @brief Handle animations
 *
 * Invoke this method periodically (such as from a VBlank/display buffer page flip handler) from
 * the UI thread to drive animations.
 */
void Screen::handleAnimations() {
    this->anim->frameCallback();
}

/**
 * @brief Update the root widget of the screen
 *
 * Replace the existing root widget with this new widget, then invalidate the screen so
 * it's redrawn.
 *
 * @param newRoot Widget to set as the root
 */
void Screen::setRootWidget(const std::shared_ptr<Widget> &newRoot) {
    if(this->rootWidget) {
        this->rootWidget->setScreen(nullptr);
        this->rootWidget.reset();
    }

    newRoot->setScreen(this->shared_from_this());
    this->rootWidget = newRoot;
    this->needsDisplay();
}

/**
 * @brief Set the root view controller
 *
 * Update the view controller that's rendered on the screen.
 *
 * @param newRoot New root view controller
 */
void Screen::setRootViewController(const std::shared_ptr<ViewController> &newRoot) {
    if(this->rootVc) {
        auto oldRoot = this->rootVc;
        oldRoot->viewWillDisappear(false);
        this->rootVc.reset();
        oldRoot->viewDidDisappear();
    }

    newRoot->viewWillAppear(false);
    this->rootVc = newRoot;

    this->setRootWidget(this->rootVc->getWidget());
    this->rootVc->viewDidAppear();
}



/**
 * @brief Process all pending events
 *
 * First, all "sum" events such as scrolling will be accumulated; then the appropriate handlers
 * for touch down/move/up and physical buttons will be invoked as well, in the order that the
 * events are received.
 */
void Screen::processEvents() {
    std::lock_guard lg(this->eventQueueLock);

    // simply clear events if events are inhibited
    if(this->eventsInhibited) {
        this->eventQueue.clear();
        return;
    }

    while(!this->eventQueue.empty()) {
        const auto &event = this->eventQueue.front();

        std::visit([&](auto&& arg) -> void {
            using T = std::decay_t<decltype(arg)>;

            /*
             * Handle touch event
             *
             * Figure out what widget is on screen at the given coordinate, then provide the event
             * to it for handling. If it doesn't handle it (or there is no widget there) the event
             * is sent to the first responder, and otherwise it's ignored.
             */
            if constexpr(std::is_same_v<T, event::Touch>) {
                // set if a touch event should result in setting a new tracking widget
                bool wantNewTrackingWidget{true};

                // check the touch tracking widget
                if(auto widget = this->touchTrackingWidget.lock()) {
                    wantNewTrackingWidget = false;

                    auto handeled = widget->handleTouchEvent(arg);
                    if(handeled) {
                        goto beach;
                    }
                }

                // identify the widget under this location
                if(this->rootWidget) {
                    Point targetPoint;
                    auto target = this->rootWidget->findChildAt(arg.position, targetPoint);

                    // try that widget
                    if(target) {
                        const auto handeled = target->handleTouchEvent(arg);
                        if(handeled) {
                            // if it handled the event, it's going to be the new tracking widget
                            if(wantNewTrackingWidget && target->wantsTouchTracking()) {
                                this->touchTrackingWidget = target;
                            }
                            goto beach;
                        }
                    }
                }

                // still unhandeled: try first responder
                if(auto widget = this->firstResponder.lock()) {
                    widget->handleTouchEvent(arg);
                }

beach:;
                // common handling
                if(!arg.isDown) {
                    this->touchTrackingWidget.reset();
                }
            }
            /*
             * Handle button event
             *
             * We'll give the current first responder widget a chance to handle the event; if it
             * doesn't handle it, we process our internal actions.
             */
            else if constexpr(std::is_same_v<T, event::Button>) {
                if(auto widget = this->firstResponder.lock()) {
                    const auto handled = widget->handleButtonEvent(arg);
                    if(handled) {
                        return;
                    }
                }

                // do our stuff
                switch(arg.type) {
                    default:
                        fprintf(stderr, "%s: unhandled, unknown button type $%02x (%s)\n",
                                "shittygui", arg.type, arg.isDown ? "down" : "up");
                }
            }
            /*
             * Handle scroll event
             *
             * This discards the return value since we don't care if the first responder didn't
             * handle the event; if it did not, there's nowhere else for it to go.
             *
             * (That is, until event bubbling is implemented…)
             */
            else if constexpr(std::is_same_v<T, event::Scroll>) {
                if(auto widget = this->firstResponder.lock()) {
                    widget->handleScrollEvent(arg);
                }
            }
        }, event);

        // go to next
        this->eventQueue.pop_front();
    }
}
