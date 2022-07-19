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
 * @brief Process an animation frame
 */
void Animator::frameCallback() {
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
