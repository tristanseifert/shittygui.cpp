#ifndef SHITTYGUI_WIDGET_H
#define SHITTYGUI_WIDGET_H

#include <memory>
#include <span>
#include <vector>

#include <shittygui/Types.h>

namespace shittygui {
/**
 * @brief Base widget class
 *
 * This is an abstract base class that all ShittyGUI widgets descend from. It provides some basic
 * common behavior for all widgets (such as bounds/origin management, event handling, view tree
 * management, etc.) under a common interface.
 */
class Widget {
    public:
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
         * @remark Even if this method returns `false` you may still receive draw events
         */
        virtual bool isDirty() = 0;

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
        virtual void draw(struct _cairo *drawCtx, const bool everything) = 0;

        void addChild(const std::shared_ptr<Widget> &toAdd);
        bool removeChild(const std::shared_ptr<Widget> &toRemove);
        bool removeFromParent();
        /**
         * @brief Get all children in this widget
         */
        constexpr inline std::span<std::shared_ptr<Widget>> getChildren() {
            return this->children;
        }
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
        std::vector<std::shared_ptr<Widget>> children;
};
};

#endif
