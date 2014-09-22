/**
 * @file fileio.hpp
 * @author Herbert Jones
 * @brief File IO errors
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <system_error>

namespace mf {
namespace utils {

enum class file_io_error
{
    UnknownError = 1,
    EndOfFile,
    StreamError,
    LineTooLong,
    NotEnoughMemory,
    BufferTooLarge,
    FileModified,
};

/**
 * @brief Create/get the instance of the error category.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category& fileio_category();

/**
 * @brief Create an error code for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error code
 */
std::error_code make_error_code(file_io_error e);

}  // End namespace utils
}  // namespace mf

namespace std {

template <>
struct is_error_code_enum<mf::utils::file_io_error> : public true_type {};

}  // namespace std
