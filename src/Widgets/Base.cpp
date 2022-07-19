#include <algorithm>
#include <functional>
#include <stdexcept>

#include <cairo.h>

#include "Animator.h"
#include "CairoHelpers.h"
#include "Errors.h"
#include "Util.h"
#include "Widget.h"

using namespace shittygui;

/**
 * @brief Add a new widget as a child
 *
 * The given widget is added to our children list, and its callbacks are invoked.
 *
 * @remark A widget hierarchy should be built from the top down. That is, create the root view
 *         first and associate it with a screen. Then, add subviews of the root view to it; then
 *         add those subviews to it, and so forth.
 *
 * @param toAdd Widget to add
 * @param atStart Insert the widget at the start of the child list (when set) rather than at the
 *        end of the list (when unset, default)instead of the end (default)
 */
void Widget::addChild(const std::shared_ptr<Widget> &toAdd, const bool atStart) {
    // validate the widget ptr
    if(!toAdd) {
        throw std::invalid_argument("invalid widget ptr");
    } else if(toAdd.get() == this) {
        throw std::invalid_argument("cannot add widget to itself");
    }

    // TODO: check whether adding it would form a loop in the widget tree
    toAdd->willMoveToParent(toAdd);

    if(atStart) {
        this->children.emplace_front(toAdd);
    } else {
        this->children.emplace_back(toAdd);
    }

    toAdd->parent = this->shared_from_this();
    toAdd->didMoveToParent();

    this->updateChildData();
}

/**
 * @brief Remove a particular child
 *
 * Remove a child widget from our hierarchy
 *
 * @param toRemove Widget to remove
 *
 * @return Whether the widget was found and removed
 */
bool Widget::removeChild(const std::shared_ptr<Widget> &toRemove) {
    using namespace std::placeholders;

    if(!toRemove) {
        throw std::invalid_argument("invalid widget ptr");
    }

    for(auto it = this->children.begin(); it != this->children.end(); ++it) {
        auto child = *it;
        if(child != toRemove) {
            continue;
        }

        // remove this bad boy
        child->willMoveToParent(nullptr);

        child->parent.reset();
        child->didMoveToParent();

        // erase the entry
        this->children.erase(it);

        this->updateChildData();
        return true;
    }

    // not found
    return false;
}

/**
 * @brief Updates cached child data after modifying the child list
 */
void Widget::updateChildData() {
    using namespace std::placeholders;

    // transparency optimizations
    this->hasTransparentChildren = !std::all_of(this->children.begin(), this->children.end(),
            std::bind(&Widget::isOpaque, _1));
    this->childrenDirtyFlag = true;
}



/**
 * @brief Draw child widgets
 *
 * Renders all children of this widget, applying the appropriate transformations and
 * clipping regions.
 *
 * This is a separate routine so that it can take place separately from the drawing of the
 * components itself: a widget will first have its draw() routine invoked, and then, if at
 * least one child exists, this routine will be invoked. This implies that the content of
 * a widget with children will be painted over by its children.
 *
 * Additionally, this routine takes into account the dirty status of children (unless the
 * `everything` flag is set) when deciding which children to draw.
 *
 * @remark This routine should be called with the coordinate space translated such that the origin
 *         of the drawing context is the same as the origin of the widget we're drawing.
 *
 * @param drawCtx Cairo drawing context
 * @param everything When set, draw everything regardless of dirty status
 */
void Widget::drawChildren(cairo_t *drawCtx, const bool everything) {
    // early abort if no children
    if(this->children.empty()) {
        return;
    }

    // translate coordinates to our origin
    cairo_save(drawCtx);

    const auto &frame = this->getFrame();
    cairo_translate(drawCtx, frame.origin.x, frame.origin.y);

    if(this->clipToBounds()) {
        cairo::Rectangle(drawCtx, this->getBounds());
        cairo_clip(drawCtx);
    }

    // process each child, in the order they were added
    for(const auto &child : this->children) {
        // skip if drawing is inhibited
        if(child->inhibitDrawing) {
            continue;
        }

        // if the child is dirty, draw it
        if(child->isDirty() || everything) {
            const auto &childFrame = child->getFrame();

            cairo_save(drawCtx);

            // clip the child to its bounds
            if(child->clipToBounds()) {
                cairo::Rectangle(drawCtx, childFrame);
                cairo_clip(drawCtx);
            }

            // translate coordinate origin
            cairo_translate(drawCtx, childFrame.origin.x, childFrame.origin.y);

            // draw the child then restore gfx state
            child->draw(drawCtx, everything);
            cairo_restore(drawCtx);
        }

        // then recurse and draw its children, if any
        child->drawChildren(drawCtx, everything);
    }

    // restore original coordinate system
    cairo_restore(drawCtx);

    this->childrenDirtyFlag = false;
}

/**
 * @brief Mark the widget as dirty
 *
 * This routine is invoked by other code in the GUI layer to mark this widget as needing
 * to be redrawn.
 *
 * @remark Subclasses must invoke the superclass' implementation of this method if they override it
 *         as it is used to propagate dirtiness up the view hierarchy.
 */
void Widget::needsDisplay() {
    if(auto ptr = this->getParent()) {
        ptr->needsChildDisplay();
    }

    this->dirtyFlag = true;
}

/**
 * @brief Indicates this widget's children need to be redrawn
 *
 * Internally invoked by the GUI layer on all ancestors of a dirty widget.
 */
void Widget::needsChildDisplay() {
    if(auto ptr = this->getParent()) {
        ptr->needsChildDisplay();
    }

    this->childrenDirtyFlag = true;
}

/**
 * This will notify our parent (or, if there is none, the screen) that it needs to redraw
 * everything.
 */
void Widget::frameDidChange() {
    if(auto parent = this->getParent()) {
        parent->childrenDirtyFlag = true;
        parent->needsDisplay();
    } else if(auto screen = this->getScreen()) {
        screen->needsDisplay();
    }
}



/**
 * @brief Set the screen the widget is on
 *
 * This is a helper method invoked by the Screen when the root view is updated. It sets the
 * reference in our instance, then invokes the appropriate handlers in all children.
 */
void Widget::setScreen(const std::shared_ptr<Screen> &newScreen) {
    using namespace std::placeholders;

    this->invokeCallbackRecursive(std::bind(&Widget::willMoveToScreen, _1, _2), newScreen);
    this->screen = newScreen;
    this->invokeCallbackRecursive(std::bind(&Widget::didMoveToScreen, _1, _2), newScreen);
}

/**
 * This default implementation registers the widget with the screen's animator.
 */
void Widget::didMoveToScreen(const std::shared_ptr<Screen> &screen) {
    if(this->wantsAnimation() && !this->animatorRegistered) {
        screen->anim->registerWidget(this->shared_from_this());
        this->animatorRegistered = true;
    }
}

/**
 * Remove the widget from the current animator as we're going to change parents soon.
 */
void Widget::willMoveToParent(const std::shared_ptr<Widget> &newParent) {
    // remove from animator
    if(auto anim = this->getAnimator()) {
        anim->unregisterWidget(this->shared_from_this());
        this->animatorRegistered = false;
    }
}

/**
 * Register the view with the animator if it wants animation.
 *
 * @remark When subclassing, you must always invoke the base widget class implementation of this
 *         method first.
 */
void Widget::didMoveToParent() {
    // install in animator
    if(this->wantsAnimation() && !this->animatorRegistered) {
        if(auto anim = this->getAnimator()) {
            anim->registerWidget(this->shared_from_this());
            this->animatorRegistered = true;
        }
    }
}



/**
 * @brief Search for a child containing the given point
 *
 * This will descend the child hierarchy to find the most specific (that is, deepest into the
 * hierarchy) widget whose frame rectangle contains this point.
 *
 * @param at Point to find the widget under
 * @param outRelativePoint Input point relative to the origin of the returned widget
 *
 * @return Widget containing the point, or `nullptr` if none
 *
 * @remark This can be slow if there are a lot of widgets.
 */
std::shared_ptr<Widget> Widget::findChildAt(const Point at, Point &outRelativePoint) {
    // bail if we don't contain this point
    if(!this->bounds.contains(at)) {
        return nullptr;
    }

    /*
     * Check all children, but in the reverse order. This is needed because widgets are drawn in
     * back to front order, so the widgets at the very end of the child list will be the visible
     * ones if there's any overlap.
     */
    for(auto it = this->children.rbegin(); it != this->children.rend(); ++it) {
        const auto &child = *it;

        // translate the point to the child's origin
        const auto &childFrame = child->getFrame();
        const auto translated = Point(at.x - childFrame.origin.x, at.y - childFrame.origin.y);

        // check this child
        auto ptr = child->findChildAt(translated, outRelativePoint);
        if(ptr) {
            return ptr;
        }
    }

    // failed to find any more specific children
    outRelativePoint = at;
    return this->shared_from_this();
}
