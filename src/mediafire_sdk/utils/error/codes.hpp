/**
 * @file codes.hpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

namespace mf {
namespace utils
{
    /** Module error codes. */
    enum class errc
    {
        UnknownError,
        EndOfFile,
        StreamError,
        LineTooLong,
        FileModified,
    };

}  // End namespace utils
}  // namespace mf
