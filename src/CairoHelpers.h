/**
 * @file
 *
 * @brief Helper methods for Cairo
 *
 * This file defines some static methods that bridge the Cairo world to some of our internal
 * types.
 */
#ifndef CAIROHELPERS_H
#define CAIROHELPERS_H

#include <numbers>

#include <cairo.h>

#include "Types.h"

/// Cairo helper methods
namespace shittygui::cairo {
/**
 * @brief Convert degrees to radians
 */
constexpr inline double DegreesToRadian(const double deg) {
    return deg * (std::numbers::pi / 180.);
}

/**
 * @brief Set the draw context's source color
 */
static inline void SetSource(cairo_t *ctx, const Color &color) {
    cairo_set_source_rgba(ctx, color.r, color.g, color.b, color.a);
}

/**
 * @brief Add a rectangle to the path
 *
 * Append to the current path the specified rectangle
 */
static inline void Rectangle(cairo_t *ctx, const Rect &rect) {
    cairo_rectangle(ctx, rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
}

/**
 * @brief Add a rounded rectangle to the path
 *
 * Append to the current path a rectangle with rounded corners. These corners are circular.
 *
 * @param rect Rectangle extents
 * @param cornerRadius Corner radius (points)
 * @param aspect Rectangle aspect ratio
 */
static inline void RoundedRect(cairo_t *ctx, const Rect &rect, const double cornerRadius,
        const double aspect = 1.) {
    // bail if it's not actually rounded
    if(cornerRadius <= 0) {
        return Rectangle(ctx, rect);
    }

    const double radius = cornerRadius / aspect;

    cairo_new_sub_path(ctx);

    cairo_arc(ctx,
            rect.origin.x + rect.size.width - radius, rect.origin.y + radius,
            radius,
            DegreesToRadian(-90), DegreesToRadian(0));
    cairo_arc(ctx,
            rect.origin.x + rect.size.width - radius, rect.origin.y + rect.size.height - radius,
            radius,
            DegreesToRadian(0), DegreesToRadian(90));
    cairo_arc(ctx,
            rect.origin.x + radius, rect.origin.y + rect.size.height - radius,
            radius,
            DegreesToRadian(90), DegreesToRadian(180));
    cairo_arc(ctx,
            rect.origin.x + radius, rect.origin.y + radius,
            radius,
            DegreesToRadian(180), DegreesToRadian(270));

    cairo_close_path(ctx);
}
}

#endif
