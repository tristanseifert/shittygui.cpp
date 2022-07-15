#include <algorithm>
#include <functional>
#include <stdexcept>

#include <cairo.h>

#include "Errors.h"
#include "Util.h"
#include "Widget.h"

using namespace shittygui;

/**
 * @brief Add a new widget as a child
 *
 * The given widget is added to our children list, and its callbacks are invoked.
 *
 * @param toAdd Widget to add
 */
void Widget::addChild(const std::shared_ptr<Widget> &toAdd) {
    if(!toAdd) {
        throw std::invalid_argument("invalid widget ptr");
    }

    toAdd->removeFromParent();
    this->children.emplace_back(toAdd);
    toAdd->adopted(this);
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

    // erase matching elements and store them
    std::vector<std::shared_ptr<Widget>> removed;
    transfer_if_not(this->children, removed, [&](auto &childPtr) {
        return (childPtr == toRemove);
    });
    // invoke the appropriate callback
    std::for_each(removed.begin(), removed.end(), std::bind(&Widget::orphaned, _1));

    return !removed.empty();
}

/**
 * @brief Remove this widget from its parent
 */
bool Widget::removeFromParent() {
    using namespace std::placeholders;

    auto ptr = this->parent.lock();
    if(!ptr) {
        return false;
    }

    std::vector<std::shared_ptr<Widget>> removed;
    transfer_if_not(ptr->children, removed, [this](auto &childPtr) {
        return (childPtr.get() == this);
    });
    std::for_each(removed.begin(), removed.end(), std::bind(&Widget::orphaned, _1));

    return !removed.empty();
}
