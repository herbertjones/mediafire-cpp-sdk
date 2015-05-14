/**
 * @file utils/container_algorithms.hpp
 * @author Herbert Jones
 * @brief Useful algorithms
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include <algorithm>
#include <functional>

namespace mf
{
namespace utils
{

/**
 * @brief Remove intersection from two sorted containers and save intersection.
 *
 * @param[in,out] sorted_a Sorted container which will have items in b removed.
 * @param[in,out] sorted_b Sorted container which will have items in a removed.
 * @param[out] sorted_b Intersection of sorted_a and sorted_b.
 */
template <typename Container,
          typename Compare = std::less<typename Container::value_type>>
void RepartitionIntersection(Container & sorted_a,
                             Container & sorted_b,
                             Container & intersection,
                             Compare compare = Compare())
{
    auto a_it = std::begin(sorted_a);
    auto a_end = std::unique(a_it, std::end(sorted_a));

    auto b_it = std::begin(sorted_b);
    auto b_end = std::unique(b_it, std::end(sorted_b));

    Container a_unique;
    Container b_unique;

    auto save_a = std::back_inserter(a_unique);
    auto save_b = std::back_inserter(b_unique);
    auto save_both = std::back_inserter(intersection);

    // Find intersection
    while (a_it != a_end && b_it != b_end)
    {
        if (*a_it == *b_it)
        {
            save_both = *a_it;
            ++a_it;
            ++b_it;
        }
        else if (compare(*a_it, *b_it))
        {
            save_a = *a_it;
            ++a_it;
        }
        else
        {
            save_b = *b_it;
            ++b_it;
        }
    }

    // Ran out of items.  Anything left is unique.
    for (; a_it != a_end; ++a_it)
        save_a = *a_it;

    for (; b_it != b_end; ++b_it)
        save_b = *b_it;

    using std::swap;
    swap(sorted_a, a_unique);
    swap(sorted_b, b_unique);
}

}  // namespace utils
}  // namespace mf
