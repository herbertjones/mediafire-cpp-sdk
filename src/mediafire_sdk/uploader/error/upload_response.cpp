/**
 * @file uploader_category.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "upload_response.hpp"

#include <sstream>
#include <string>

namespace {
class UploadResponseCategory : public std::error_category
{
public:
    /// The name of this error category.
    virtual const char* name() const NOEXCEPT;

    /// The message belonging to the error code.
    virtual std::string message(int ev) const;
};

const char* UploadResponseCategory::name() const NOEXCEPT
{
    return "upload response";
}

std::string UploadResponseCategory::message(int ev) const
{
    std::stringstream ss;
    ss << "Upload response error: " << ev;
    return ss.str();
}

}  // namespace

const std::error_category& mf::uploader::upload_response_category()
{
    static UploadResponseCategory instance;
    return instance;
}
