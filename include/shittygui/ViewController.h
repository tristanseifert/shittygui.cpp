#ifndef SHTITYGUI_VIEWCONTROLLER_H
#define SHTITYGUI_VIEWCONTROLLER_H

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <memory>
#include <string_view>
#include <vector>

namespace shittygui {
class Widget;

/**
 * @brief View Controller
 *
 * View controllers implement the "controller" part of the traditional MVC paradigm. They manage
 * the lifecycle of an underlying root widget (the "view") and receive certain notifications from
 * the UI system.
 */
class ViewController: public std::enable_shared_from_this<ViewController> {
    friend class Animator;

    public:
        /**
         * @brief View controller animation types
         *
         * A view controller may be presented/dismissed with animation. In this case, one of these
         * animation styles may be applied.
         */
        enum class PresentationAnimation: uint8_t {
            /// No animation takes place
            None,

            /**
             * @brief Slide up
             *
             * The target view controller slides into view from the bottom of the screen.
             */
            SlideUp,
        };

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

    public:
        /**
         * @brief Present a view controller
         *
         * @param vc View controller to present
         * @param isAnimated Whether the default animation is applied
         */
        inline void presentViewController(const std::shared_ptr<ViewController> &vc,
                const bool isAnimated) {
            this->presentViewController(vc,
                    isAnimated ? PresentationAnimation::SlideUp : PresentationAnimation::None);
        }
        void presentViewController(const std::shared_ptr<ViewController> &vc,
                const PresentationAnimation anim);

        /**
         * @brief Dismiss any presented view controller
         *
         * Dismiss the currently presented view controller, animating it if requested with the
         * default animation.
         *
         * @remark Invoke this on the _presenting_ view controller to dismiss.
         *
         * @seeAlso dismiss
         */
        inline void dismissViewController(const bool isAnimated) {
            this->dismissViewController(
                    isAnimated ? PresentationAnimation::SlideUp : PresentationAnimation::None);
        }
        void dismissViewController(const PresentationAnimation anim);

        void dismiss(const bool isAnimated);

    protected:
        /**
         * @brief Get the presenting view controller
         */
        inline std::shared_ptr<ViewController> getParent() {
            return this->parent.lock();
        }

    private:
        void dismissFinalize();

        void startAnimating();
        void endAnimating();

        bool processAnimationFrame();

    private:
        /**
         * @brief Time duration for an animated presentation (seconds)
         */
        constexpr static const double kPresentationAnimationDuration{.35};

    private:
        /**
         * @brief Parent (presenting) view controller
         *
         * This is the view controller from which we've been presented. This may be `nullptr` if
         * this is the root view controller, or if we haven't been presented.
         */
        std::weak_ptr<ViewController> parent;

        /**
         * @brief Presented view controller
         *
         * The view controller that is currently being presented, if any.
         */
        std::shared_ptr<ViewController> presenting;

        /**
         * @brief Child widgets at presentation time
         *
         * A list of all widgets that are children of our root widget, present at the time that a
         * child controller is presented.
         *
         * This list is kept such that we can make all these widgets inhibit their rendering.
         */
        std::vector<std::weak_ptr<Widget>> presentedWidgets;

        /**
         * @brief Animation state
         *
         * This is all the state used by the presentation/dismissal animation handling.
         */
        struct {
            /// When was the animation started?
            std::chrono::high_resolution_clock::time_point start;
            /// Callback token
            uint32_t token;

            /// What type of animation is used?
            PresentationAnimation type{PresentationAnimation::None};

            /// Is an animation in progress?
            uint8_t isActive                    :1{false};
            /// Whether the animation is for presentation (`true`) or dismissal (`false`)
            uint8_t presentation                :1{false};
        } animation;
};
}

#endif
