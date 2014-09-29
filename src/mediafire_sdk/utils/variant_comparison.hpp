/**
 * @file variant_equals.hpp
 * @author Herbert Jones
 * @brief Helper class for boost::variant compares
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <functional>

#include "boost/variant/apply_visitor.hpp"
#include "boost/variant/static_visitor.hpp"

namespace mf {
namespace utils {

/**
 * @class VariantEquals
 * @brief Compare different types
 *
 * Used to compare two different types as false, or compare if types same.
 * Useful for comparing variants.
 */
template<template<typename> class Compare>
class VariantCompareVisitor : public boost::static_visitor<bool>
{
public:
    template <typename T, typename U>
    bool operator()( const T &, const U & ) const
    {
        return false; // cannot compare different types
    }

    template <typename T>
    bool operator()( const T & lhs, const T & rhs ) const
    {
        return Compare<T>()(lhs, rhs);
    }
};

/**
 * @brief Compare two boost::variants using the visitor pattern.
 *
 * @param[in] lhs Variant to compare.
 * @param[in] rhs Variant to compare.
 *
 * @return True if Compare true on inputs.
 */
template<typename T, template<typename> class Compare>
bool VariantCompare(const T & lhs, const T & rhs)
{
    return boost::apply_visitor( VariantCompareVisitor<Compare>(), lhs, rhs );
}

/**
 * @brief Compare two boost::variants using the visitor pattern.
 *
 * @param[in] lhs Variant to compare.
 * @param[in] rhs Variant to compare.
 *
 * @return True if inputs equal.
 */
template<typename T>
bool AreVariantsEqual(const T & lhs, const T & rhs)
{
    return VariantCompare<T, std::equal_to>(lhs, rhs);
}

}  // namespace utils
}  // namespace mf
