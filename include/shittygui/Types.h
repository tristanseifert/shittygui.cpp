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

    /// Is this color opaque?
    constexpr inline bool isOpaque() const {
        return (this->a >= 1.);
    }

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

/**
 * @brief Rectangular area
 *
 * Defines a rectangular area on the screen, by a combination of its origin point and the full size
 * of the region.
 *
 * @remark The origin is located at the top left of the screen.
 */
struct Rect {
    Rect() = default;
    constexpr Rect(const Point origin, const Size size) : origin(origin), size(size) {}

    /**
     * @brief Inset the rectangle by the given amount
     *
     * The rectangle's origin is shifted by the given X and Y amounts, and its size decreased by
     * double these values.
     *
     * @remark Specify negative inset values to expand the rect instead.
     *
     * @param dX Inset on X axis
     * @param dY Inset on Y axis
     */
    constexpr inline Rect inset(const double dX, const double dY) const {
        auto temp = *this;
        temp.origin.x += dX;
        temp.origin.y += dY;
        temp.size.width -= (dX * 2.);
        temp.size.height -= (dY * 2.);
        return temp;
    }
    /**
     * @brief Inset the rectangle by the same amount in both directions
     *
     * @param d Inset value for both X and Y
     */
    constexpr inline Rect inset(const double d) const {
        return this->inset(d, d);
    }

    /**
     * @brief Test if the given point lies inside the rectangle
     *
     * @param p Point to test
     *
     * @return Whether the point lies inside the rect
     */
    constexpr inline bool contains(const Point p) const {
        const auto x2 = this->origin.x + this->size.width,
              y2 = this->origin.y + this->size.height;
        return p.x >= this->origin.x && p.x <= x2 && p.y >= this->origin.y && p.y <= y2;
    }

    Point origin;
    Size size;
};

/**
 * @brief Ellipsization mode
 *
 * Ellipsization is the process of inserting an ellipsis character (…) in a string of text that is
 * too large to fit in the alotted space.
 */
enum class EllipsizeMode: uint8_t {
    /// Do not insert an ellipsis anywhere
    None,
    /// Omit characters at the beginning of the text
    Start,
    /// Omit characters in the middle of the text
    Middle,
    /// Omit characters at the end of the text
    End,
};

/**
 * @brief Text layout alignment (horizontal)
 */
enum class TextAlign: uint8_t {
    Left,
    Center,
    Right,
};

/**
 * @brief Text layout alignment (vertical)
 */
enum class VerticalAlign {
    Top,
    Middle,
    Bottom
};

}

#endif
