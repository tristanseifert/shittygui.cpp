#ifndef SHITTYGUI_ANIMATOR_H
#define SHITTYGUI_ANIMATOR_H

#include <list>
#include <functional>
#include <memory>
#include <unordered_map>

namespace shittygui {
class Widget;
class Screen;

/**
 * @brief Animation manager
 */
class Animator {
    friend class Screen;

    public:
        /// Return `false` to remove the callback
        using Callback = std::function<bool(void)>;

        Animator(Screen *);
        ~Animator();

        void registerWidget(const std::shared_ptr<Widget> &widget);
        void unregisterWidget(const std::shared_ptr<Widget> &widget);

        uint32_t registerCallback(const Callback &callback);
        void unregisterCallback(const uint32_t token);

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

        /// Callbacks
        std::unordered_map<uint32_t, Callback> callbacks;
        /// Next callbacck token
        uint32_t nextToken{0};
};
}

#endif
