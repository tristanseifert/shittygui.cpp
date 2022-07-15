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
 * @brief A color value (with alpha)
 *
 * Color values are stored as floating point, with a nominal range of [0, 1]. Lower or higher
 * bounds may be supported by the underlying renderer (for example, by high color depth displays)
 * but this is not guaranteed.
 */
struct Color {
    /// Create a fully opaque black color
    Color() = default;
    /// Create a fully opaque RGB color
    constexpr Color(const float r, const float g, const float b) : r(r), g(g), b(b) {}
    /// Create a color with the specified alpha value
    constexpr Color(const float r, const float g, const float b, const float a) : r(r), g(g), b(b), a(a) {}

    /// Color component values
    float r{0}, g{0}, b{0};
    /// Alpha component value
    float a{1.};
};

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
