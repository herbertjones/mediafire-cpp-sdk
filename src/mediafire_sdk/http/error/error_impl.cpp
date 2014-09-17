/**
 * @file error_impl.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "error_impl.hpp"


std::error_condition mf::http::make_error_condition(errc e)
{
    return std::error_condition(
            static_cast<int>(e),
            mf::http::error_category()
            );
}

std::error_code mf::http::make_error_code(errc e)
{
    return std::error_code(
            static_cast<int>(e),
            mf::http::error_category()
            );
}
