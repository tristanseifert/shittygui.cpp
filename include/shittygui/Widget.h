#ifndef SHITTYGUI_WIDGET_H
#define SHITTYGUI_WIDGET_H

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include <shittygui/Event.h>
#include <shittygui/Screen.h>
#include <shittygui/Types.h>

namespace shittygui {
class Animator;
class Screen;

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
class Widget: public std::enable_shared_from_this<Widget> {
    friend class Screen;
    friend class ViewController;

    public:
        /**
         * @brief Event handler type
         *
         * Widgets should use this type of function for their event callbacks. The first argument
         * will always be a pointer to the widget from the event.
         */
        using EventCallback = std::function<void(const std::shared_ptr<Widget> &sender)>;

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
         * @brief Set whether the view is hidden
         */
        inline void setHidden(const bool hidden) {
            this->hidden = hidden;
            this->needsDisplay();
        }
        /**
         * @brief Get whether the view is hidden
         */
        constexpr inline bool isHidden() const {
            return this->hidden;
        }


        /**
         * @brief Does the widget need to be redrawn?
         *
         * Widgets should keep track of their internal "dirty" state. When their state changes,
         * they should mark themselves as dirty (and update parents)
         *
         * This basic implementation uses a boolean flag that's set by `needsDisplay` as well as
         * if any child widgets become dirty.
         *
         * @remark Even if this method returns `false` you may still receive draw events
         */
        virtual bool isDirty() {
            return this->dirtyFlag || this->childrenDirtyFlag;
        }

        virtual void needsDisplay();
        virtual void needsChildDisplay();

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
        /**
         * @brief Remove the widget from its parent
         */
        bool removeFromParent() {
            auto parent = this->getParent();
            if(!parent) {
                return false;
            }

            return parent->removeChild(this->shared_from_this());
        }

        /**
         * @brief Whether this view has any children
         */
        constexpr inline bool hasChildren() {
            return !this->children.empty();
        }

        /**
         * @brief Invoked when the widget is about to move to a new parent
         *
         * @param parent Pointer to the widget's new parent
         */
        virtual void willMoveToParent(const std::shared_ptr<Widget> &newParent) {}
        /**
         * @brief Invoked when the widget moved to a new parent
         */
        virtual void didMoveToParent() {}

        /**
         * @brief The frame rectangle of the widget has changed
         *
         * @remark Subclasses must invoke this base class implementation.
         */
        virtual void frameDidChange();

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
            this->frameDidChange();
        }
        /**
         * @brief Set the origin of the frame rectangle
         */
        void setFrameOrigin(const Point newOrigin) {
            this->frame.origin = newOrigin;
            this->needsDisplay();
            this->frameDidChange();
        }
        /**
         * @brief Get the bounds rectangle of the widget
         */
        constexpr inline auto getBounds() const {
            return this->bounds;
        }

        /**
         * @brief Convert a rectangle to screen space
         *
         * @param in Input rectangle, in the coordinate space of this widget
         */
        inline Rect convertToScreenSpace(const Rect &in) {
            auto temp = in;

            // offset this by our frame
            temp.origin.x += this->frame.origin.x;
            temp.origin.y += this->frame.origin.y;

            // recurse to the parent implementation of this
            if(auto parent = this->getParent()) {
                return parent->convertToScreenSpace(temp);
            } else {
                return temp;
            }
        }

        /**
         * @brief Notification that our root view is about to change screens
         */
        virtual inline void willMoveToScreen(const std::shared_ptr<Screen> &newScreen) {}

        /**
         * @brief Notification that the root screen changed
         *
         * All widgets receive this notification when the screen that their root view is attached
         * to changes.
         *
         * @param newScreen Reference to the new screen
         *
         * @remark Subclasses must invoke this base class implementation.
         */
        virtual void didMoveToScreen(const std::shared_ptr<Screen> &newScreen) {};

    public:
        /**
         * @brief Whether the widget accepts user input
         *
         * User input in this case refers to scroll and keyboard events. A widget typically
         * acquires user interface focus by being clicked (where it can then request as such from
         * the screen) but it may also be selected by default.
         *
         * @seeAlso hasDefaultFocus
         */
        virtual inline bool acceptsUserInput() {
            return false;
        }

        /**
         * @brief Whether the widget should receive focus immediately
         *
         * When this is set, the widget will be set as the focussed widget when it's added to a
         * screen.
         *
         * @remark Only one widget in the entire hierarchy should have this flag set; otherwise
         *         the behavior is undefined.
         */
        virtual inline bool hasDefaultFocus() {
            return false;
        }

        /**
         * @brief Whether the widget wants to track touch events
         *
         * When a widget opts in to tracking of touch events, it will receive all subsequent touch
         * events after a touch down (until the touch is released) event, even if they exit the
         * bounds of the widget.
         */
        virtual inline bool wantsTouchTracking() {
            return false;
        }

        /**
         * @brief Handle a touch event
         *
         * These events are received by the component under the touch point first; if it does not
         * process the event, they'll go to the first responder.
         *
         * You can acquire first responder status in this callback if desired to receive all
         * subsequent events.
         */
        virtual inline bool handleTouchEvent(const event::Touch &event) {
            return false;
        }

        /**
         * @brief Handle a scroll event
         *
         * Scroll events are typically generated by scroll wheels or encoders. They have an
         * associated relative delta value.
         *
         * @return Whether the event was handeled
         */
        virtual inline bool handleScrollEvent(const event::Scroll &event) {
            return false;
        }

        /**
         * @brief Handle a button event
         *
         * @return Whether the event was handeled
         */
        virtual inline bool handleButtonEvent(const event::Button &event) {
            return false;
        }

        /**
         * @brief Set the debug label of the widget
         */
        inline void setDebugLabel(const std::string_view &newLabel) {
            this->debugLabel = newLabel;
        }
        /**
         * @brief Get a debug label for the component
         *
         * This is a string label that can be dumped for debugging to identify a component.
         */
        constexpr inline std::string_view getDebugLabel() const {
            return this->debugLabel;
        }

        /**
         * @brief Set the tag
         */
        inline void setTag(const uintptr_t newTag) {
            this->tag = newTag;
        }
        /**
         * @brief Get the widget's tag
         */
        constexpr inline auto getTag() const {
            return this->tag;
        }

        /**
         * @brief Apply a method on all child widgets
         */
        template<typename Func, typename... Args>
        inline void forEachChild(Func what, Args&&... args) {
            for(auto &child : this->children) {
                what(child, std::forward<Args>(args)...);
            }
        }

    protected:
        /**
         * @brief Get the parent of this widget
         *
         * If the widget is the root widget, it will have no parent. Instead, its screen pointer is
         * set to the screen it is on, if any.
         */
        inline std::shared_ptr<Widget> getParent() {
            return this->parent.lock();
        }

        /**
         * @brief Find the animator responsible for this widget's screen
         *
         * Locate the screen this widget is on, then return its animator. The animator is used by widgets
         * which want to animate their display to receive periodic callbacks.
         */
        inline std::shared_ptr<Animator> getAnimator() {
            auto screen = this->getScreen();
            if(!screen) {
                return nullptr;
            }

            return screen->getAnimator();
        }

        /**
         * @brief Get the screen this widget is currently on
         */
        inline std::shared_ptr<Screen> getScreen() {
            // get the root widget
            if(auto parent = this->getParent()) {
                std::shared_ptr<Widget> next = parent;

                do {
                    parent = next;
                    next = next->getParent();
                } while(next);

                return parent->getScreen();
            }
            // we're the root, so return the screen ptr
            else {
                return this->screen.lock();
            }
        }

        std::shared_ptr<Widget> findChildAt(const Point at, Point &outRelativePoint);

    private:
        void setScreen(const std::shared_ptr<Screen> &newScreen);

        void updateChildData();

        /**
         * @brief Execute a widget callback (recursive step)
         *
         * @tparam Func Function type for the callback (typically an std::bind expression)
         * @tparam Args Argument pack for the callback function
         */
        template<typename Func, typename... Args>
        void invokeCallbackRecursive(std::list<std::shared_ptr<Widget>> &widgets, Func what, Args&&... args) {
            for(auto &child : widgets) {
                what(child, std::forward<Args>(args)...);

                if(!child->children.empty()) {
                    this->invokeCallbackRecursive(child->children, what, std::forward<Args>(args)...);
                }
            }
        }

        /**
         * @brief Execute a widget callback
         *
         * Invoke the specified callback function on this widget and all of its children
         * recursively.
         *
         * @tparam Func Function type for the callback (typically an std::bind expression)
         * @tparam Args Argument pack for the callback function
         *
         * @param what Callback function to invoke
         * @param args Arguments to callback function
         */
        template<typename Func, typename... Args>
        void invokeCallbackRecursive(Func what, Args&&... args) {
            what(this, std::forward<Args>(args)...);
            this->invokeCallbackRecursive(this->children, what, std::forward<Args>(args)...);
        }

    protected:
        /**
         * @brief Debugging label string
         */
        std::string debugLabel;

        /// User specified tag value
        uintptr_t tag{0};

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
        uintptr_t dirtyFlag                     :1{false};

        /**
         * @brief Children need redraw
         *
         * Set when any of this widget's children become dirtied. This flag is propagated up from
         * the widget that dirtied; that is, all ancestors of that widget will have this flag set.
         */
        uintptr_t childrenDirtyFlag             :1{false};

        /**
         * @brief Are any children not opaque?
         *
         * This flag is updated any time children are added or removed from this widget. It's used
         * to optimize drawing.
         */
        uintptr_t hasTransparentChildren        :1{false};

        /**
         * @brief Set to inhibit any drawing of this widget
         *
         * This is set by view controllers during animations; for example, to prevent any animated
         * widgets _under_ the presented view from rendering.
         */
        uintptr_t inhibitDrawing                :1{false};

        /**
         * @brief Set when the widget is participating in animation
         *
         * During animation, we want to always redraw all widgets, regardless of their dirty state
         */
        uintptr_t animationParticipant          :1{false};

        /**
         * @brief Is the view hidden?
         *
         * Hidden views will not be drawn.
         */
        uintptr_t hidden                        :1{false};

    private:
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
         * @brief Current screen
         *
         * Set for the root widget in a widget hierarchy when it's added to a screen.
         */
        std::weak_ptr<Screen> screen;

        /**
         * @brief Child widgets
         *
         * Pointers to all children added to widget.
         */
        std::list<std::shared_ptr<Widget>> children;
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
