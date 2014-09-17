/**
 * @file upload_modification.hpp
 * @author Herbert Jones
 * @brief Upload modification request types
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "boost/variant/variant.hpp"

namespace mf {
namespace uploader {

namespace modification {
struct Cancel {};
struct Pause {};
}  // namespace modification

using UploadModification = boost::variant
    < modification::Cancel
    , modification::Pause
    >;

}  // namespace uploader
}  // namespace mf
