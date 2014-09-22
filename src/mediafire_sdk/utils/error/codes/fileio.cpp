/**
 * @file fileio.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "fileio.hpp"

#include <string>
#include <sstream>

#include "mediafire_sdk/utils/noexcept.hpp"

namespace {

class CategoryImpl
    : public std::error_category
{
public:
  virtual const char* name() const NOEXCEPT;
  virtual std::string message(int ev) const;
  virtual std::error_condition default_error_condition(int ev) const NOEXCEPT;
};

const char* CategoryImpl::name() const NOEXCEPT
{
    return "mediafire utils fileio";
}

std::string CategoryImpl::message(int ev) const
{
    using mf::utils::file_io_error;

    switch (static_cast<file_io_error>(ev))
    {
        case file_io_error::UnknownError:
            return "unknown error";
        case file_io_error::EndOfFile:
            return "end of file";
        case file_io_error::StreamError:
            return "stream error";
        case file_io_error::LineTooLong:
            return "line too long";
        case file_io_error::NotEnoughMemory:
            return "not enough memory";
        case file_io_error::BufferTooLarge:
            return "buffer too large";
        default:
        {
            std::ostringstream ss;
            ss << "Unknown error: " << ev;
            return ss.str();
        }
    }
}

std::error_condition CategoryImpl::default_error_condition(
        int ev
    ) const NOEXCEPT
{
    using mf::utils::file_io_error;

    switch (static_cast<file_io_error>(ev))
    {
        //case file_io_error::UnknownError:
        //    return std::errc::
        //case file_io_error::EndOfFile:
        //    return std::errc::
        case file_io_error::StreamError:
            return std::errc::io_error;
        //case file_io_error::LineTooLong:
        //    return std::errc::
        case file_io_error::NotEnoughMemory:
            return std::errc::not_enough_memory;
        //case file_io_error::BufferTooLarge:
        //    return std::errc::
        default:
            return std::error_condition(ev, *this);
    }
}

}  // namespace

namespace mf {
namespace utils {

const std::error_category& fileio_category()
{
    static CategoryImpl instance;
    return instance;
}

std::error_code make_error_code(file_io_error e)
{
    return std::error_code(
            static_cast<int>(e),
            mf::utils::fileio_category()
            );
}

}  // End namespace utils
}  // namespace mf
