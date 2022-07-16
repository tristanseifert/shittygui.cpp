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

#include <cairo.h>

#include "Types.h"

/// Cairo helper methods
namespace shittygui::cairo {
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
}

#endif
