//
//  thread_id_impl.hpp
//
//
//  Created by Zach Marquez on 2/12/14.
//  @copyright Copyright 2014 MediaFire, LLC.
//
// Contains private functions related to native thread ids.
#pragma once

#include <string>

#include "thread_id.hpp"

namespace mf {
namespace utils {
namespace detail {

// Tells the OS to use threadName for the current thread's name.
void SetNativeThreadName(const std::string& threadName);

// Queries the OS for the current thread's name.
std::string GetNativeThreadName();

}  // namespace detail
}  // namespace utils
}  // namespace mf
