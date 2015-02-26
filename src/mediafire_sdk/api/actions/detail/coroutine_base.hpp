/**
 * @file api/actions/detail/coroutine_base.hpp
 * @author Herbert Jones
 * @brief Implementaion for coroutine base.
 * @copyright Copyright 2015 Mediafire
 *
 * Non-templated base for coroutines to provide common operations on unchanging
 * types.
 */
#pragma once

#include <system_error>

#include "boost/optional.hpp"

#include "mediafire_sdk/api/types.hpp"

namespace mf
{
namespace api
{
namespace detail
{

class CoroutineBase
{
public:
    std::error_code GetErrorCode() const { return error_code_; }
    boost::optional<std::string> GetErrorDescription() const
    {
        return error_description_;
    }

    boost::optional<ActionResult> GetActionResult() const
    {
        return action_result_;
    }

protected:
    std::error_code error_code_;
    boost::optional<std::string> error_description_;
    boost::optional<ActionResult> action_result_;
};

}  // namespace detail
}  // namespace api
}  // namespace mf
