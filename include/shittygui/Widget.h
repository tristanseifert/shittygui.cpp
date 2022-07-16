#ifndef SHITTYGUI_WIDGET_H
#define SHITTYGUI_WIDGET_H

#include <deque>
#include <memory>
#include <utility>

#include <shittygui/Types.h>

namespace shittygui {
/**
 * @brief Base widget class
 *
 * This is an abstract base class that all ShittyGUI widgets descend from. It provides some basic
 * common behavior for all widgets (such as bounds/origin management, event handling, view tree
 * management, etc.) under a common interface.
 *
 * Widgets have both bounds and a frame; this is similar to other GUI frameworks. In our case, the
 * bounds define the size of a widget in its own coordinate system (that is, its origin coincides
 * with the origin of the widget.) On the other hand, the frame specifies an origin relative to the
 * parent of the widget.
 */
class Widget {
    public:
        /**
         * @brief Initialize a widget with the given frame
         *
         * @param frame Frame rectangle, its origin is relative to the parent widget
         */
        Widget(const Rect &frame) {
            this->setFrame(frame);
        }

        virtual ~Widget() = default;

        /**
         * @brief Determine whether the widget is fully opaque
         *
         * Opaque widgets can use an optimized drawing path, so you should always make a widget
         * opaque if possible.
         */
        virtual bool isOpaque() {
            return true;
        }

        /**
         * @brief Should drawing be clipped to the bounds of the widget?
         *
         * The parent widget can add a clip region to the widget to ensure nothing gets drawn
         * outside the bounds of the widget if desired.
         */
        virtual bool clipToBounds() {
            return true;
        }

        /**
         * @brief Does the widget need to be redrawn?
         *
         * Widgets should keep track of their internal "dirty" state. When their state changes,
         * they should mark themselves as dirty (and update parents)
         *
         * This basic implementation uses a boolean flag that's set by `needsDisplay`.
         *
         * @remark Even if this method returns `false` you may still receive draw events
         */
        virtual bool isDirty() {
            return this->dirtyFlag;
        }

        /**
         * @brief Mark the widget as dirty
         *
         * This routine is invoked by other code in the GUI layer to mark this widget as needing
         * to be redrawn.
         */
        virtual void needsDisplay();

        /**
         * @brief Draw the widget
         *
         * Based on the return value of clipToBounds(), this method will be invoked with a clip
         * region set up to cover the bounds of this view. Additionally, the drawing context will
         * be translated such that its origin is the same as the screen origin of this widget.
         *
         * @param drawCtx Cairo drawing context
         * @param everything When set, draw everything regardless of dirty status
         */
        virtual void draw(struct _cairo *drawCtx, const bool everything = false) {
            this->dirtyFlag = false;
        }
        virtual void drawChildren(struct _cairo *drawCtx, const bool everything = false);

        void addChild(const std::shared_ptr<Widget> &toAdd, const bool atStart = false);
        bool removeChild(const std::shared_ptr<Widget> &toRemove);
        bool removeFromParent();

        /**
         * @brief Whether this view has any children
         */
        constexpr inline bool hasChildren() {
            return !this->children.empty();
        }

        /**
         * @brief Invoked when the widget is added to a view hierarchy
         *
         * @param newParent New parent widget
         */
        virtual void adopted(Widget *newParent) {}
        /**
         * @brief Invoked when the widget is removed from the view hierarchy
         */
        virtual void orphaned() {}

        /**
         * @brief Get the frame rectangle of the widget
         */
        constexpr inline auto getFrame() const {
            return this->frame;
        }
        /**
         * @brief Set the frame rectangle of the widget
         *
         * Set the frame (where the origin is _relative_ to its parent) of the widget.
         *
         * @param newFrame New frame rectangle
         */
        void setFrame(const Rect &newFrame) {
            this->frame = newFrame;
            this->bounds = {{0, 0}, newFrame.size};
            this->needsDisplay();
        }
        /**
         * @brief Get the bounds rectangle of the widget
         */
        constexpr inline auto getBounds() const {
            return this->bounds;
        }

    protected:
        /**
         * @brief Parent widget
         *
         * The parent widget is the widget that contains this one. This may be null if this widget
         * has not been added to the view hierarchy, or if it's the root widget.
         *
         * Keep a weak reference as to prevent retain cycles.
         */
        std::weak_ptr<Widget> parent;

        /**
         * @brief Child widgets
         *
         * Pointers to all children added to widget.
         */
        std::deque<std::shared_ptr<Widget>> children;

        /**
         * @brief Frame rectangle
         */
        Rect frame;
        /**
         * @brief Bounds rectangle
         */
        Rect bounds;

        /**
         * @brief Dirty indicator
         *
         * Set any time needsDisplay is called, and returned by isDirty. If you're using the
         * default dirty tracking, reset this flag when done with draw().
         *
         * @seeAlso needsDisplay
         * @seeAlso isDirty
         */
        bool dirtyFlag{false};
};

/**
 * @brief Allocate a new widget
 *
 * @tparam T Widget type
 * @tparam Args... Additional arguments for widget constructor
 *
 * @param origin Origin of the widget's frame
 * @param size Size of the widget's frame
 *
 * @return Initialized widget
 */
template<typename T, typename... Args>
std::shared_ptr<T> MakeWidget(const Point origin, const Size size, Args&&... args) {
    return std::make_shared<T>(Rect{origin, size}, std::forward<Args>(args)...);
}

/// Widget implementations
namespace widgets {

};
};

#endif
