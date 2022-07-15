/**
 * @file
 *
 * @brief Utility and helper functions
 */
#ifndef SHITTYGUI_UTIL_H
#define SHITTYGUI_UTIL_H

#include <algorithm>
#include <vector>

namespace shittygui {
/**
 * @brief Move elements between vectors based on predicate
 *
 * @tparam T Element type in vectors
 * @tparam F Predicate type
 *
 * @param old Input vector containing all elements
 * @param out Output vector to receive all elements matching predicate
 * @parap pred Predicate to invoke to decide what elements to move
 */
template <class T, class F>
void transfer_if_not(std::vector<T> &old, std::vector<T> &out, F pred) {
    auto part = std::partition(old.begin(), old.end(), pred);
    std::move(part, old.end(), std::back_inserter(out));
    old.erase(part);
}
}

#endif
