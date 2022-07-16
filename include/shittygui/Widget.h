#ifndef SHITTYGUI_WIDGET_H
#define SHITTYGUI_WIDGET_H

#include <deque>
#include <memory>
#include <utility>

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
         * @brief Does the widget desire animation?
         *
         * When set, the widget is opted into animation support. This means it may receive a
         * callback after every frame is rendered from the animator to update its state.
         */
        virtual bool wantsAnimation() {
            return false;
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

        /**
         * @brief Process an animation frame
         *
         * This is invoked by the animator any time a frame is rendered.
         *
         * @seeAlso Screen::handleAnimations()
         */
        virtual void processAnimationFrame() {}

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
        virtual void adopted(const std::shared_ptr<Widget> &newParent);
        /**
         * @brief Invoked when the widget is removed from the view hierarchy
         */
        virtual void orphaned();

        /**
         * @brief The frame rectangle of the widget has changed
         */
        virtual void frameDidChange() {}

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
         * @brief Get the bounds rectangle of the widget
         */
        constexpr inline auto getBounds() const {
            return this->bounds;
        }

        /**
         * @brief The widget is moving to a new screen
         *
         * @remark This routine is invoked only on the root widget. It is useless in any child
         *         widgets.
         */
        inline void moveToScreen(const std::shared_ptr<Screen> &newScreen) {
            this->screen = newScreen;
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

    private:
        void updateChildData();

    protected:
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
        std::deque<std::shared_ptr<Widget>> children;
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
