/**
 * @file types.hpp
 *
 * @copyright Copyright 2014 Mediafire
 *
 * Collection of useful typedefs for the project.
 */
#pragma once

#include <cstdint>

namespace mf {

/** Type for storing file sizes. */
typedef uint64_t FileSize;

/** Marker to indicate invalid filesize. */
static const FileSize InvalidFileSize = UINT64_MAX;

}  // namespace mf
