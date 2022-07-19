#ifndef SHITTYGUI_EASINGFUNCTIONS_H
#define SHITTYGUI_EASINGFUNCTIONS_H

#include <cmath>

namespace shittygui {
/**
 * @brief Various easing functions
 *
 * See [easings.net](https://easings.net) for an interactive demonstration.
 */
struct EasingFunctions {
    constexpr static inline double InOutQuad(double t) {
        return t < 0.5 ? 2. * t * t : t * (4. - 2. * t) - 1.;
    }

    constexpr static inline double InOutCubic(double t) {
        double t2{t};
        return t2 < 0.5 ? 4. * t2 * t2 * t2 : 1. + (t2 - 1.) * (2. * (t2 - 2.)) * (2. * (t2 - 3.));
    }

    constexpr static inline double InOutQuart(double t) {
        if(t < 0.5) {
            t *= t;
            return 8. * t * t;
        } else {
            --t;
            t = t * t;
            return 1. - 8. * t * t;
        }
    }

    constexpr static inline double InOutCirc(double t) {
        if(t < 0.5) {
            return (1. - sqrtf(1. - 2. * t)) * 0.5;
        } else {
            return (1. + sqrtf(2. * t - 1.)) * 0.5;
        }
    }

    constexpr static inline double InOutElastic(double t) {
        double t2;
        if(t < 0.45) {
            t2 = t * t;
            return 8. * t2 * t2 * sinf(t * (double)M_PI * 9.);
        } else if(t < 0.55) {
            return 0.5 + 0.75 * sinf(t * (double)M_PI * 4.);
        } else {
            t2 = (t - 1.) * (t - 1.);
            return 1. - 8. * t2 * t2 * sinf(t * (double)M_PI * 9.);
        }
    }
};
}

#endif
