#ifndef SHTITYGUI_VIEWCONTROLLER_H
#define SHTITYGUI_VIEWCONTROLLER_H

#include <memory>
#include <string_view>

namespace shittygui {
class Widget;

/**
 * @brief View Controller
 *
 * View controllers implement the "controller" part of the traditional MVC paradigm. They manage
 * the lifecycle of an underlying root widget (the "view") and receive certain notifications from
 * the UI system.
 */
class ViewController {
    public:
        virtual ~ViewController() = default;

        /**
         * @brief Get the root widget
         *
         * @remark Changing the widget once the view controller has been presented is not
         *         supported and will lead to unpredictable behavior.
         */
        virtual std::shared_ptr<Widget> &getWidget() = 0;

        /**
         * @brief Title for the view controller
         *
         * This title may be displayed in the user interface, for example in a navigation bar in a
         * navigation controller.
         */
        virtual std::string_view getTitle() {
            return "";
        }

    public:
        /**
         * @brief View controller is about to be made visible
         *
         * This is invoked immediately before any presentation animations are invoked.
         *
         * @remark You should always invoke this superclass implementation if overriding
         *
         * @param isAnimated Whether the view controller is presented with animation
         */
        virtual void viewWillAppear(const bool isAnimated) {}

        /**
         * @brief View controller is fully visible
         *
         * If the view controller was presented with animation, this indicates that all animations
         * have been completed.
         *
         * @remark You should always invoke this superclass implementation if overriding
         */
        virtual void viewDidAppear() {}

        /**
         * @brief View controller is about to be made invisible
         *
         * The view controller is about to be removed from the display.
         *
         * @remark You should always invoke this superclass implementation if overriding
         *
         * @param isAnimated Whether the view controller will be removed with animation
         */
        virtual void viewWillDisappear(const bool isAnimated) {}

        /**
         * @brief View controller has become invisible
         *
         * View controller is now fully off-screen; if the disappearance was animated, this means
         * that all animations have completed.
         *
         * @remark You should always invoke this superclass implementation if overriding
         */
        virtual void viewDidDisappear() {}
};
}

#endif
