/**
 * @file timeouts.hpp
 * @author Herbert Jones
 * @brief Timeout contants
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

namespace {
const int kResolvingTimeout = 30;
const int kSslHandshakeTimeout = 30;
const int kConnectTimeout = 30;
const int kProxyWriteTimeout = 30;
const int kProxyReadTimeout = 30;

}  // namespace
