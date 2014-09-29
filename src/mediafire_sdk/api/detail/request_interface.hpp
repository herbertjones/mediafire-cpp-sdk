/**
 * @file request_interface.hpp
 * @author Herbert Jones
 * @brief Wrapper for API requests.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <memory>

namespace mf {
namespace api {
namespace detail {

/**
 * @interface RequestInterface
 * @brief Interface for abstracting access to API requests.
 */
class RequestInterface
{
public:
    virtual ~RequestInterface() {}

    /**
     * @brief Cancel the operation if possible.
     *
     * If operation already complete cancel is ignored.
     */
    virtual void Cancel() = 0;

private:
};

}  // namespace detail
}  // namespace api
}  // namespace mf
