#include <stdexcept>
#include <unordered_set>

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

    std::unordered_set<uint32_t> toRemove;

    for(const auto &[token, callback] : this->callbacks) {
        const bool cont = callback();
        if(!cont) {
            toRemove.insert(token);
        }
    }

    for(const auto token : toRemove) {
        this->unregisterCallback(token);
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

/**
 * @brief Register an animation callback
 *
 * @return Token used to remove the callback later
 */
uint32_t Animator::registerCallback(const Callback &callback) {
    uint32_t token;

    // get an unused token
    do {
        token = ++this->nextToken;
    } while(!token || this->callbacks.contains(token));

    this->callbacks.emplace(token, callback);

    return token;
}

/**
 * @brief Remove a previously registered animation callback
 */
void Animator::unregisterCallback(const uint32_t token) {
    this->callbacks.erase(token);
}
