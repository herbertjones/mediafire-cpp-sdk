/**
 * @file time.hpp
 * @author Herbert Jones
 * @brief Time functions
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>

#include <ctime>

#include "boost/optional.hpp"
#include "boost/date_time/posix_time/ptime.hpp"

namespace mf {
namespace utils {

/**
 * @brief Convert a ptime value to a time_t
 *
 * @param[in] pt ptime to convert
 *
 * @return Epoch seconds if conversion possible.
 */
boost::optional<std::time_t> TimeFromPtime(const boost::posix_time::ptime & pt);

/**
 * @brief Convert a string value to a time_t
 *
 * The time should match the pattern: 2000-01-01 20:00:00.000
 *
 * @param[in] str String to convert
 *
 * @return Epoch seconds if conversion possible.
 */
boost::optional<std::time_t> TimeFromString(const std::string & str);

}  // namespace utils
}  // namespace mf
