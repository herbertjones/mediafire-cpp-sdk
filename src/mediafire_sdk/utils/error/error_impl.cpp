/**
 * @file error_impl.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "error_impl.hpp"


std::error_condition mf::utils::make_error_condition(errc e)
{
    return std::error_condition(
            static_cast<int>(e),
            mf::utils::error_category()
            );
}

std::error_code mf::utils::make_error_code(errc e)
{
    return std::error_code(
            static_cast<int>(e),
            mf::utils::error_category()
            );
}
