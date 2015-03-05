/**
 * @file utils/lambda_visitor.hpp
 * @brief Useful util for dealing with visitor pattern.
 * @copyright Copyright 2015 Mediafire
 *
 * As seen on Stack Overflow
 */
#pragma once

#include "boost/variant/apply_visitor.hpp"
#include "boost/variant/static_visitor.hpp"

namespace mf
{
namespace utils
{

template <typename ReturnT, typename... Lambdas>
struct lambda_visitor;

/**
 * @brief Build a struct with
 *
 * Long description...
 *
 * @param[in] type Description
 *
 * @return Return description
 */
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

template <typename ReturnT, typename... Lambdas>
lambda_visitor<ReturnT, Lambdas...> make_lambda_visitor(Lambdas... lambdas)
{

    return {lambdas...};
}

}  // namespace utils
}  // namespace mf
