#ifndef SHITTYGUI_ERRORS_H
#define SHITTYGUI_ERRORS_H

#include <stdexcept>

#include <cairo.h>

namespace shittygui {
/**
 * @brief Throw an exception corresponding to the given Cairo status code
 *
 * @param status Cairo status code (should be an error type)
 */
inline void ThrowForCairoStatus(const cairo_status_t status) {
    switch(status) {
        case CAIRO_STATUS_NO_MEMORY:
            throw std::runtime_error("out of memory");
        case CAIRO_STATUS_NULL_POINTER:
            throw std::runtime_error("NULL pointer specified");
        case CAIRO_STATUS_READ_ERROR:
            throw std::runtime_error("read error");
        case CAIRO_STATUS_INVALID_CONTENT:
            throw std::runtime_error("invalid content");
        case CAIRO_STATUS_INVALID_FORMAT:
            throw std::runtime_error("invalid format");
        case CAIRO_STATUS_INVALID_VISUAL:
            throw std::runtime_error("invalid visual");

        default:
            throw std::runtime_error("unknown cairo error");
        case CAIRO_STATUS_SUCCESS:
            break;
    }
}
}

#endif
