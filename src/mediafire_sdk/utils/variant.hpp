/**
 * @file utils/variant.hpp
 * @brief Useful util for dealing with variants.
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include "detail/variant.hpp"

namespace mf
{
namespace utils
{

/**
 * @brief Make visitor for boost::variant by series of lambdas.
 *
 * Keep logic near apply_visitor by placing the lambdas to process a variant
 * with the function that iterates its types.
 *
 * @template ReturnT Return type
 * @param[in] lambdas Lambdas that handle all the types of the variant they
 *                    expect the process.
 *
 * @return New visitor for passing to apply_visitor.
 */
template <typename ReturnT, typename... Lambdas>
detail::lambda_visitor<ReturnT, Lambdas...> make_lambda_visitor(
        Lambdas... lambdas)
{
    return {lambdas...};
}

/**
 * @brief Act upon a variant type.
 *
 * @warning All handlers must have the same return type.
 * @warning There must be an handler for all possible types in variant.
 *
 * @param[in] variant boost::variant to pass to type handlers.
 * @param[in] lambdas Functions to handle all possible types stored in variant.
 *
 * @return The result of the selected lambda.
 */
template <typename VariantT, typename Lambda1, typename... Lambdas>
typename detail::return_trait<Lambda1>::result_type
Match(const VariantT variant, Lambda1 lambda1, Lambdas &&... lambdas)
{
    auto visitor = make_lambda_visitor<
            typename detail::return_trait<decltype(lambda1)>::result_type>(
            lambda1, lambdas...);

    return boost::apply_visitor(visitor, variant);
}

}  // namespace utils
}  // namespace mf
