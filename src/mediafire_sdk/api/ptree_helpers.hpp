/**
 * @file ptree_helpers.hpp
 * @author Herbert Jones
 * @brief Helpers for working with boost property trees.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>

#include "boost/property_tree/ptree.hpp"
#include "boost/optional.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"

#include "mediafire_sdk/utils/string.hpp"

namespace mf {
namespace api {

/**
 * @brief Set the passed in variable if the specified property exists.
 *
 * @param[in] pt Property tree with possible value.
 * @param[in] property_path The path of the property. The path separator is a
 * dot(.).
 * @param[out] to_set Variable to set with expected property data.
 *
 * @return true if set occurred.
 */
template<typename T>
bool GetIfExists(
        const boost::property_tree::wptree & pt,
        std::string property_path,
        T * to_set
    )
{
    const std::wstring property_path_w = mf::utils::bytes_to_wide(property_path);
    boost::optional<T> result = pt.get_optional<T>(property_path_w);
    if ( result )
    {
        *to_set = *result;
        return true;
    }
    return false;
}

/**
 * @brief Set the passed in variable if the specified property exists.
 *
 * std::string override. String might accept an empty value, which isn't ideal.
 * Override with length check. Other types should fail during type cast if value
 * is empty.
 *
 * @param[in] pt Property tree with possible value.
 * @param[in] property_path The path of the property. The path separator is a
 * dot(.).
 * @param[out] to_set Variable to set with expected property data.
 *
 * @return true if set occurred.
 */
bool GetIfExists(
        const boost::property_tree::wptree & pt,
        std::string property_path,
        std::string * to_set
    );

/**
 * @brief Set the passed in variable if the specified property exists.
 *
 * boost::posix_time::ptime override
 *
 * @param[in] pt Property tree with possible value.
 * @param[in] property_path The path of the property. The path separator is a
 * dot(.).
 * @param[out] to_set Variable to set with expected property data.
 *
 * @return true if set occurred.
 */
bool GetIfExists(
        const boost::property_tree::wptree & pt,
        std::string property_path,
        boost::posix_time::ptime * to_set
    );

/**
 * @brief Set the passed in variable if the specified property is an array and
 * contains at least one element.
 *
 * @param[in] pt Property tree with possible value in an array.
 * @param[in] property_path The path of the property array. The path separator
 *            is a dot(.).
 * @param[out] to_set Variable to set with expected property data.
 *
 * @return true if set occurred.
 */
template<typename T>
bool GetIfExistsArrayFront(
        const boost::property_tree::wptree & pt,
        std::string property_path,
        T * to_set
    )
{
    try {
        const std::wstring property_path_w = mf::utils::bytes_to_wide(property_path);

        const boost::property_tree::wptree & child =
            pt.get_child(property_path_w);

        for ( const auto & it : child )
        {
            boost::optional<T> result = it.second.get_value_optional<T>();
            if ( result )
            {
                *to_set = *result;
                return true;
            }
            return false;
        }
    }
    catch(boost::property_tree::ptree_bad_path & err)
    {}
    return false;
}

/**
 * @brief Set the passed in variable if the specified property is an array and
 * contains at least one element.
 *
 * @param[in] pt Property tree with possible value in an array.
 * @param[in] property_path The path of the property array. The path separator
 *            is a dot(.).
 * @param[out] to_set Variable to set with expected property data.
 *
 * @return true if set occurred.
 */
bool GetIfExistsArrayFront(
        const boost::property_tree::wptree & pt,
        std::string property_path,
        std::string * to_set
    );

/**
 * @brief Set the passed in variable if the specified ptree has a value
 * convertible to T.
 *
 * @param[in] pt Property tree with possible value.
 *
 * @tparam T The type to convert the ptree to.
 *
 * @param[out] to_set Variable to set with expected property data.
 *
 * @return true if set occurred.
 */
template<typename T>
bool GetValueIfExists(
        const boost::property_tree::wptree & pt,
        T * to_set
    )
{
    boost::optional<T> result = pt.get_value_optional<T>();
    if ( result )
    {
        *to_set = *result;
        return true;
    }
    return false;
}

template <>
bool GetValueIfExists<std::string>(
        const boost::property_tree::wptree & pt,
        std::string * to_set
    );

/**
 * @brief True if property path matches passed in variable.
 *
 * @param[in] pt Property tree with possible value.
 * @param[in] property_path The path of the property. The path separator is a
 * dot(.).
 * @param[in] value Variable with expected property data.
 *
 * @return true if property path exists with expected data.
 */
template<typename T>
bool PropertyHasValue(
        const boost::property_tree::wptree & pt,
        std::string property_path,
        T value
    )
{
    const std::wstring property_path_w = mf::utils::bytes_to_wide(property_path);

    boost::optional<T> result = pt.get_optional<T>(property_path_w);
    if ( result )
    {
        return value == *result;
    }
    return false;
}

/**
 * @brief True if property path matches passed in variable.
 *
 * @param[in] pt Property tree with possible value.
 * @param[in] property_path The path of the property. The path separator is a
 * dot(.).
 * @param[in] value Variable with expected property data.
 *
 * @return true if property path exists with expected data.
 */
bool PropertyHasValue(
        const boost::property_tree::wptree & pt,
        std::string property_path,
        std::string value
    );

/**
 * @brief True if property path matches passed in string, matched case
 * insensitively.
 *
 * @param[in] pt Property tree with possible value.
 * @param[in] property_path The path of the property. The path separator is a
 * dot(.).
 * @param[in] value Variable with expected property data.
 *
 * @return true if property path exists with expected string.
 */
bool PropertyHasValueCaseInsensitive(
        const boost::property_tree::wptree & pt,
        std::string property_path,
        std::string value
    );

}  // namespace api
}  // namespace mf

