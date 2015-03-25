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
    auto visitor = detail::make_lambda_visitor<
            typename detail::return_trait<decltype(lambda1)>::result_type>(
            lambda1, lambdas...);

    return boost::apply_visitor(visitor, variant);
}

/**
 * @brief Act upon a variant type.  Match only supplied handlers.
 *
 * Ignores any variants that do not match supplied handlers.  Returns void as
 * there is no default value.
 *
 * @warning All handlers must have void return type.
 *
 * @param[in] variant boost::variant to pass to type handlers.
 * @param[in] lambdas Functions to handle all possible types stored in variant.
 */
template <typename VariantT, typename... Lambdas>
void MatchPartial(const VariantT variant, Lambdas &&... lambdas)
{
    auto visitor = detail::make_partial_lambda_visitor(lambdas...);

    boost::apply_visitor(visitor, variant);
}

/**
 * @brief Act upon a variant type.  Match only supplied handlers.  Recurse on
 *        inner variants.
 *
 * Ignores any variants that do not match supplied handlers.  Returns void as
 * there is no default value.
 *
 * @warning All handlers must have void return type.
 *
 * @param[in] variant boost::variant to pass to type handlers.
 * @param[in] lambdas Functions to handle all possible types stored in variant.
 */
template <typename VariantT, typename... Lambdas>
void MatchPartialRecursive(const VariantT variant, Lambdas &&... lambdas)
{
    auto visitor = detail::make_partial_recursive_lambda_visitor(lambdas...);

    boost::apply_visitor(visitor, variant);
}

/**
 * @brief Act upon a variant type.  Match only supplied handlers.
 *
 * Ignores any variants that do not match supplied handlers.  Returns default
 * value if no handlers match variant.
 *
 * @warning All handlers must have same return type as default value.
 *
 * @param[in] variant boost::variant to pass to type handlers.
 * @param[in] default_value Value returned when no supplied handler matches
 *                          variant.
 * @param[in] lambdas Functions to handle all possible types stored in variant.
 *
 * @return The result of the selected lambda or default value if no lambda
 *         matched variant type.
 */
template <typename VariantT, typename ReturnT, typename... Lambdas>
ReturnT MatchPartialWithDefault(const VariantT variant,
                                const ReturnT & default_value,
                                Lambdas &&... lambdas)
{
    auto visitor = detail::make_partial_lambda_visitor_with_default(
            default_value, lambdas...);
    return boost::apply_visitor(visitor, variant);
}

/**
 * @brief Act upon a variant type.  Match only supplied handlers.  Recurse on
 *        inner variants.
 *
 * Ignores any variants that do not match supplied handlers.  Returns default
 * value if no handlers match variant.
 *
 * @warning All handlers must have same return type as default value.
 *
 * @param[in] variant boost::variant to pass to type handlers.
 * @param[in] default_value Value returned when no supplied handler matches
 *                          variant.
 * @param[in] lambdas Functions to handle all possible types stored in variant.
 *
 * @return The result of the selected lambda or default value if no lambda
 *         matched variant type.
 */
template <typename VariantT, typename ReturnT, typename... Lambdas>
ReturnT MatchPartialRecursiveWithDefault(const VariantT variant,
                                         const ReturnT & default_value,
                                         Lambdas &&... lambdas)
{
    auto visitor = detail::make_partial_recursive_lambda_visitor_with_default(
            default_value, lambdas...);
    return boost::apply_visitor(visitor, variant);
}

}  // namespace utils
}  // namespace mf
