/**
 * @file utils/detail/variant.hpp
 * @brief Implementation for variant helpers.
 * @copyright Copyright 2015 Mediafire
 *
 * Parts taken from Stack Overflow
 */
#pragma once

#include <algorithm>
#include <type_traits>

#include "boost/variant/apply_visitor.hpp"
#include "boost/variant/static_visitor.hpp"

namespace mf
{
namespace utils
{
namespace detail
{

template <typename ReturnT, typename... Lambdas>
struct lambda_visitor;

template <typename ReturnT, typename L1, typename... Lambdas>
struct lambda_visitor<ReturnT, L1, Lambdas...>
        : public L1, public lambda_visitor<ReturnT, Lambdas...>

{
    using L1::operator();

    using lambda_visitor<ReturnT, Lambdas...>::operator();

    lambda_visitor(L1 l1, Lambdas... lambdas)

            : L1(l1), lambda_visitor<ReturnT, Lambdas...>(lambdas...)

    {
    }
};

template <typename ReturnT, typename L1>
struct lambda_visitor<ReturnT, L1> : public L1,
                                     public boost::static_visitor<ReturnT>

{
    using L1::operator();

    lambda_visitor(L1 l1)

            : L1(l1), boost::static_visitor<ReturnT>()

    {
    }
};

template <typename ReturnT>
struct lambda_visitor<ReturnT> : public boost::static_visitor<ReturnT>
{
    lambda_visitor() : boost::static_visitor<ReturnT>() {}
};

template <typename T>
struct return_trait : public return_trait<decltype(&T::operator())>
{
};
// For generic types, directly use the result of the signature of its
// 'operator()'

template <typename ClassType, typename ReturnType, typename... Args>
struct return_trait<ReturnType (ClassType::*)(Args...) const>
// we specialize for pointers to member function
{
    typedef ReturnType result_type;
};

}  // namespace detail
}  // namespace utils
}  // namespace mf
