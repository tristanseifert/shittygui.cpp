#include <chrono>
#include <functional>
#include <stdexcept>

#include "Animator.h"
#include "Widget.h"
#include "Screen.h"
#include "ViewController.h"

using namespace shittygui;

/**
 * @brief Present a view controller
 *
 * The specified view controller will be displayed (by adding it to our widget hierarchy) and
 * possibly animated.
 *
 * @param vc View controller to display
 * @param anim Animation to use
 */
void ViewController::presentViewController(const std::shared_ptr<ViewController> &vc,
        const PresentationAnimation anim) {
    if(!vc) {
        throw std::invalid_argument("invalid view controller");
    }

    // ensure we're not already presenting
    if(this->presenting) {
        throw std::runtime_error("Already presenting a view controller!");
    }

    vc->parent = this->shared_from_this();
    vc->viewWillAppear((anim != PresentationAnimation::None));

    // inhibit rendering on our widgets
    auto &ourWidget = this->getWidget();

    this->presentedWidgets.clear();
    this->presentedWidgets.reserve(ourWidget->children.size());

    for(const auto &widget : ourWidget->children) {
        this->presentedWidgets.emplace_back(widget);
    }

    // dope; figure out the root widget's frame
    const auto &ourBounds = ourWidget->getBounds();
    auto &widget = vc->getWidget();
    auto widgetFrame = widget->getBounds();

    switch(anim) {
        // start off entirely outside the bounds of the view
        case PresentationAnimation::SlideUp:
            widgetFrame.origin.y = ourBounds.size.height;
            break;

        // the widget is just shown normally
        case PresentationAnimation::None:
            break;
    }

    // insert it into the hierarchy and kick off animation if needed
    widget->setFrame(widgetFrame);

    ourWidget->addChild(widget);

    this->animation.type = anim;
    this->animation.presentation = true;

    this->presenting = vc;

    // if not animating, invoke the "did appear" callback
    if(anim == PresentationAnimation::None) {
        vc->viewDidAppear();

        // also ensure the widgets below stop rendering immediately
        for(const auto &ptr : this->presentedWidgets) {
            if(auto widget = ptr.lock()) {
                widget->inhibitDrawing = true;
            }
        }
    }
    // otherwise, begin animation processing
    else {
        this->startAnimating();
    }
}



/**
 * @brief Dismiss this view controller
 *
 * If this view controller was presented from another view controller, request that it be
 * dismissed.
 *
 * @param isAnimated Whether the dismissal will be animated
 *
 * @throw std::runtime_error If we were not presented on a view controller
 */
void ViewController::dismiss(const bool isAnimated) {
    auto parent = this->getParent();
    if(!parent) {
        throw std::runtime_error("View controller must be presented");
    }

    parent->dismissViewController(isAnimated);
}

/**
 * @brief Dismiss the currently presented view controller
 *
 * @param anim Animation to use for the dismissal
 */
void ViewController::dismissViewController(const PresentationAnimation anim) {
    using namespace std::placeholders;

    // ensure we're already presenting
    if(!this->presenting) {
        throw std::runtime_error("Not presenting a view controller!");
    }

    // prepare for disappearance
    this->presenting->viewWillDisappear((anim != PresentationAnimation::None));

    this->animation.type = anim;
    this->animation.presentation = false;

    // restart drawing of the previously inhibited widgets
    for(auto &ptr : this->presentedWidgets) {
        if(auto widget = ptr.lock()) {
            widget->inhibitDrawing = false;

            // recursively redraw them - this is a little agressive
            widget->invokeCallbackRecursive(std::bind(&Widget::needsDisplay, _1));
        }
    }

    this->presentedWidgets.clear();

    this->getWidget()->needsDisplay();

    // if not animating, invoke the "did disappear" callback
    if(anim == PresentationAnimation::None) {
        this->dismissFinalize();
    }
    // otherwise, begin animation processing
    else {
        this->startAnimating();
    }
}

/**
 * @brief Final step of view controller dismissal
 *
 * This is invoked after the dismissal animation is complete, or immediately if there is no
 * animation planned. It removes the presented view controller's root widget, and then runs its
 * remaining callbacks.
 */
void ViewController::dismissFinalize() {
    // remove the widget
    auto removed = this->presenting->getWidget()->removeFromParent();
    if(!removed) {
        throw std::logic_error("failed to remove presented view controller view");
    }

    // invoke callbacks
    this->presenting->viewDidDisappear();

    // clear stored references
    this->presenting->parent.reset();
    this->presenting.reset();
}



/**
 * @brief Set up for an animated presentation
 *
 * This registers an animation callback with the screen so that we receive animation events, which
 * we then use to actually drive the animations (by changing frames)
 */
void ViewController::startAnimating() {
    auto screen = this->getWidget()->getScreen();
    if(!screen) {
        throw std::logic_error("cannot present with animation on off-screen view controller!");
    }

    this->animation.token = screen->getAnimator()->registerCallback([&]() -> bool {
        return this->processAnimationFrame();
    });

    // internal bookkeeping
    this->animation.isActive = true;
    this->animation.start = std::chrono::high_resolution_clock::now();
}

/**
 * @brief Finish an animated presentation
 *
 * Stop receiving animation callbacks.
 */
void ViewController::endAnimating() {
    auto screen = this->getWidget()->getScreen();
    if(!screen) {
        throw std::logic_error("cannot present with animation on off-screen view controller!");
    }

    // internal bookkeeping
    this->animation.isActive = false;
}

/**
 * @brief Drive the animation
 *
 * Perform the required actions for a step in the presentation animation. It's done in terms of
 * a percentage between the start and now.
 *
 * @return Whether animation shall continue
 */
bool ViewController::processAnimationFrame() {
    // calculate percentage
    const auto now = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> diff = now - this->animation.start;
    const auto percent = std::min(diff.count() / kPresentationAnimationDuration, 1.);

    /*
     * Update the presenting view's frame with the animation progress
     */
    const auto &ourBounds = this->getWidget()->getBounds();
    auto &widget = this->presenting->getWidget();
    auto widgetFrame = widget->getBounds();

    switch(this->animation.type) {
        case PresentationAnimation::SlideUp:
            widgetFrame.origin.y = static_cast<double>(ourBounds.size.height) * (1. - percent);
            break;

        // should really never happen
        case PresentationAnimation::None:
            goto done;
    }

    widget->setFrame(widgetFrame);

    /*
     * If the end of the animation is reached, stop requesting updates and normalize the state of
     * the view controller.
     *
     * This includes invoking callbacks: if we're disappearing, ensure that callback is run, then
     * the view controller is removed from references as well as its root widget is removed from
     * our widget hierarchy.
     */
    if(percent >= 1.) {
done:;
        this->endAnimating();

        if(this->animation.presentation) {
            this->presenting->viewDidAppear();

            // stop widgets from rendering through
            for(const auto &ptr : this->presentedWidgets) {
                if(auto widget = ptr.lock()) {
                    widget->inhibitDrawing = true;
                }
            }
        } else {
            this->dismissFinalize();
        }

        // XXX: make sure the frame is properly set

        return false;
    }

    return true;
}
