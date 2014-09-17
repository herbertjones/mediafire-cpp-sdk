/**
 * @file upload_target.hpp
 * @author Herbert Jones
 * @brief Container for upload target folder
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>

#include "boost/variant/variant.hpp"

namespace mf {
namespace uploader {
namespace detail {

struct ParentFolderKey
{
    std::string key;
};

struct CloudPath
{
    std::string path;
};

using UploadTarget = boost::variant
    < ParentFolderKey
    , CloudPath
    >;

}  // namespace detail
}  // namespace uploader
}  // namespace mf
