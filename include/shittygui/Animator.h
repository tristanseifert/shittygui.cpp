#ifndef SHITTYGUI_ANIMATOR_H
#define SHITTYGUI_ANIMATOR_H

#include <list>
#include <memory>

namespace shittygui {
class Widget;
class Screen;

/**
 * @brief Animation manager
 */
class Animator {
    friend class Screen;

    public:
        Animator(Screen *);
        ~Animator();

        void registerWidget(const std::shared_ptr<Widget> &widget);
        void unregisterWidget(const std::shared_ptr<Widget> &widget);

    private:
        void frameCallback();

    private:
        struct WidgetInfo {
            /// Pointer to the widget
            std::weak_ptr<Widget> ptr;
        };

        /// Screen instance that created us
        Screen *owner{nullptr};

        /// All widgets that want to be animated
        std::list<WidgetInfo> widgets;
};
}

#endif