#include <stdexcept>

#include <cairo.h>

#include "Animator.h"
#include "CairoHelpers.h"
#include "Errors.h"
#include "Screen.h"
#include "Widget.h"

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
        this->rootWidget->drawChildren(this->drawCtx, this->forceDisplayFlag);

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
