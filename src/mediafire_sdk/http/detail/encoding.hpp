/**
 * @file encoding.hpp
 * @author Herbert Jones
 * @brief Encoding types
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

namespace mf {
namespace http {
namespace detail {

enum TransferEncoding
{
    TE_None          =  0,
    TE_Unknown       = (1<<0),

    TE_ContentLength = (1<<1),
    TE_Chunked       = (1<<2),
    TE_Gzip          = (1<<3),
};

enum ContentEncoding
{
    CE_None    =  0,
    CE_Unknown = (1<<0),

    CE_Gzip    = (1<<1),
};

}  // namespace detail
}  // namespace http
}  // namespace mf
