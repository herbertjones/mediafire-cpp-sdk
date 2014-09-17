/**
 * @file time.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "time.hpp"

#include "boost/date_time.hpp"

boost::optional<std::time_t> mf::utils::TimeFromPtime(
        const boost::posix_time::ptime & pt
    )
{
    if( pt == boost::posix_time::not_a_date_time )
        return boost::optional<std::time_t>();

    boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1));
    boost::posix_time::time_duration::sec_type x = (pt - epoch).total_seconds();

    return std::time_t(x);
}

boost::optional<std::time_t> mf::utils::TimeFromString(
        const std::string & timestamp
    )
{
    boost::posix_time::ptime pt = boost::posix_time::time_from_string(timestamp);
    return TimeFromPtime(pt);
}
