/**
 * @file api_category.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "api_category.hpp"

#include <sstream>
#include <string>

namespace mf {
namespace api {

/**
 * @class ApiCategoryImpl
 * @brief std::error_category implementation for api namespace
 */
class ApiCategoryImpl : public std::error_category
{
public:
    /// The name of this error category.
    virtual const char* name() const NOEXCEPT;

    /// The message belonging to the error code.
    virtual std::string message(int ev) const;
};

const char* ApiCategoryImpl::name() const NOEXCEPT
{
    return "api error code";
}

std::string ApiCategoryImpl::message(int ev) const
{
    std::stringstream ss;
    ss << "API error code: " << ev;
    return ss.str();
}

const std::error_category& api_category()
{
    static ApiCategoryImpl instance;
    return instance;
}

}  // End namespace api
}  // namespace mf
