/**
 * @file
 *
 * @brief Base types used by ShittyGUI library
 */
#ifndef SHITTYGUI_TYPES_H
#define SHITTYGUI_TYPES_H

#include <cstddef>
#include <cstdint>

namespace shittygui {
/**
 * @brief Size of an object (in pixels)
 */
struct Size {
    /// Create a size set to zero
    Size() = default;
    /**
     * @brief Create a new size
     *
     * @param width How wide is it?
     * @param height How tall is it?
     */
    constexpr Size(const uint16_t width, const uint16_t height) : width(width), height(height) {}

    /// Width of screen, in pixels
    uint16_t width{0};
    /// Height of screen, in pixels
    uint16_t height{0};
};

/**
 * @brief Point on screen
 */
struct Point {
    Point() = default;
    constexpr Point(const int16_t x, const int16_t y) : x(x), y(y) {}

    int16_t x{0};
    int16_t y{0};
};
}

#endif
