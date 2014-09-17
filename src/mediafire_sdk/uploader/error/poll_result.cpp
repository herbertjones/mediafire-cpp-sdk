/**
 * @file poll_result.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "poll_result.hpp"

#include <sstream>
#include <string>

namespace {
/**
 * @class PollResultCategory
 * @brief std::error_category implementation for uploader namespace
 */
class PollResultCategory : public std::error_category
{
public:
    /// The name of this error category.
    virtual const char* name() const NOEXCEPT;

    /// The message belonging to the error code.
    virtual std::string message(int ev) const;
};

const char* PollResultCategory::name() const NOEXCEPT
{
    return "poll result";
}

std::string PollResultCategory::message(int ev) const
{
    std::stringstream ss;
    ss << "Poll result error: " << ev;
    return ss.str();
}
}  // namespace

const std::error_category& mf::uploader::poll_result_category()
{
    static PollResultCategory instance;
    return instance;
}
