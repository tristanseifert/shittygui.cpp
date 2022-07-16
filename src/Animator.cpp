#include <stdexcept>

#include "Animator.h"
#include "Screen.h"
#include "Widget.h"

using namespace shittygui;

/**
 * @brief Initialize a new animator instance
 *
 * @param screen The screen that owns this animator
 */
Animator::Animator(Screen *screen) : owner(screen) {

}

/**
 * @brief Clean up animator resources
 */
Animator::~Animator() {

}

/**
 * @brief Process an animation frame
 */
void Animator::frameCallback() {
    for(const auto &info : this->widgets) {
        if(auto widget = info.ptr.lock()) {
            widget->processAnimationFrame();
        }
    }
}

/**
 * @brief Register a widget for animation callbacks
 */
void Animator::registerWidget(const std::shared_ptr<Widget> &widget) {
    // prepare info structure
    WidgetInfo info{
        .ptr = widget
    };

    // insert it
    this->widgets.push_back(std::move(info));
}

/**
 * @brief Remove a widget from animator purview
 *
 * The provided widget may have been previously registered by registerWidget(); if it was never
 * registered, this operation is a no-op.
 */
void Animator::unregisterWidget(const std::shared_ptr<Widget> &widget) {
    std::erase_if(this->widgets, [&](auto &info) {
        auto ptr = info.ptr.lock();
        if(!ptr) {
            return true;
        }

        return (ptr.get() == widget.get());
    });
}

