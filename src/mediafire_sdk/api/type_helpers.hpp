/**
 * @file type_helpers.hpp
 * @author Herbert Jones
 * @brief Collection of function for type conversion in API requests.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <set>
#include <vector>

#include "boost/date_time/posix_time/posix_time.hpp"

#include "mediafire_sdk/utils/string.hpp"

// No namespace here to prevent overriding AsString in anonymous namespaces.

// Forward declarations --------------------------------------------------------

/**
 * @brief Convert value to string for use in APIs.
 *
 * @param[in] value Value to convert
 *
 * @return Value as string.
 */
template<typename T>
std::string AsString(const T & value);

/**
 * @brief Convert value to string for use in APIs.
 *
 * Override for ptime values.
 *
 * @param[in] value Value to convert
 *
 * @return Value as string.
 */
std::string AsString(const boost::posix_time::ptime & value);

/**
 * @brief Convert value to string for use in APIs.
 *
 * Override for string values.
 *
 * @param[in] value Value to convert
 *
 * @return Value as string.
 */
std::string AsString(const std::string & value);

// Helpers ---------------------------------------------------------------------

/**
 * @struct AsStringHelper
 * @brief Static struct used to unpack values for passing to AsString.
 */
template<typename T> struct AsStringHelper
{
    /** Performs conversion */
    static std::string Convert(const T & t)
    {
        return mf::utils::to_string(t);
    }
};

/**
 * @brief Helper for converting containers of values via AsString.
 *
 * @param[in] v Container of values to convert to string.
 *
 * @return Values as string.
 */
template<typename T>
std::string AsStringContainerType(const T & v)
{
    std::string str;
    if ( ! v.empty() )
    {
        const std::string separator(",");
        typename T::const_iterator it(v.begin());
        str += AsString(*it);
        for (++it; it != v.end(); ++it)
        {
            str += separator;
            str += AsString(*it);
        }
    }

    return str;
}

/**
 * @struct AsStringHelper
 * @brief Static struct used to unpack values for passing to AsString.
 */
template<typename T> struct AsStringHelper< std::vector<T> >
{
    /** Performs conversion */
    static std::string Convert(const std::vector<T> & t)
    {
        return AsStringContainerType(t);
    }
};

/**
 * @struct AsStringHelper
 * @brief Static struct used to unpack values for passing to AsString.
 */
template<typename T> struct AsStringHelper< std::set<T> >
{
    /** Performs conversion */
    static std::string Convert(const std::set<T> & t)
    {
        return AsStringContainerType(t);
    }
};

// AsString --------------------------------------------------------------------

template<typename T>
std::string AsString(const T & t)
{
    return AsStringHelper<T>::Convert(t);
}

