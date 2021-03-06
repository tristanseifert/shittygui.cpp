#ifndef SHITTYGUI_EVENT_H
#define SHITTYGUI_EVENT_H

#include <cstddef>
#include <cstdint>
#include <variant>

#include <shittygui/Types.h>

namespace shittygui {
namespace event {
/**
 * @brief Touch event
 *
 * Indicates that a touch event took place. These are emitted any time a touch is down, when it
 * moves while down, and when it is released again.
 */
struct Touch {
    /**
     * @brief Create a touch event at the given location on screen
     *
     * @param position Center point of the touch
     * @param isDown Whether the touch is currently down or released
     */
    constexpr Touch(const Point position, const bool isDown) : position(position), isDown(isDown){}

    /// Touch position on screen
    Point position;
    /// Is the touch event currently pressed down?
    bool isDown{false};
};

/**
 * @brief Scroll event
 *
 * Scroll events can be generated by things such as hardware encoders.
 */
struct Scroll {
    Scroll() = default;
    /**
     * @brief Create a new scroll event with the given scroll delta
     */
    constexpr Scroll(int delta) : delta(delta) {}

    /**
     * @brief Scroll delta
     *
     * This is the number of scroll "steps" that have elapsed since the last event. These values
     * have no defined meaning, other than that negative values mean scrolling up/left, and
     * positive values down/right.
     */
    int delta{0};
};

/**
 * @brief Button events
 *
 * Hardware buttons on the system can generate additional events to the GUI system. Hardware may
 * have more buttons than this, but these are the only ones the GUI system cares about and
 * handles.
 */
struct Button {
    /// Button type
    enum Type: uint8_t {
        /// Selection button (encoder middle)
        Select                          = (1 << 0),
        /// Menu (or back) button
        Menu                            = (1 << 1),
    };

    /**
     * @brief Create a new button event
     */
    constexpr Button(const Type type, const bool isDown) : type(type), isDown(isDown) {}

    /// Button type
    Type type;
    /// Whether the button was pressed or released
    bool isDown{false};
};
}

/**
 * @brief GUI event
 *
 * Encapsulation for all supported input events
 */
using Event = std::variant<std::monostate, event::Touch, event::Scroll, event::Button>;
}

#endif
