/**
 * @file type_helpers.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "type_helpers.hpp"

std::string AsString(const boost::posix_time::ptime & value)
{
    if(value != boost::posix_time::not_a_date_time)
        return boost::posix_time::to_iso_extended_string(value) + "Z";
    return "";
}

std::string AsString(const std::string & value)
{
    return value;
}
