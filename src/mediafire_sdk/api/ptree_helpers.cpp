/**
 * @file ptree_helpers.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "ptree_helpers.hpp"

#include <string>

#include "boost/algorithm/string/predicate.hpp"  // iequals
#include "boost/date_time/local_time_adjustor.hpp"

namespace api = mf::api;

bool api::PropertyHasValueCaseInsensitive(
        const boost::property_tree::wptree & pt,
        std::string property_path,
        std::string value
    )
{
    const std::wstring property_path_w = mf::utils::bytes_to_wide(property_path);

    boost::optional<std::wstring> result =
        pt.get_optional<std::wstring>(property_path_w);

    if ( result )
    {
        return boost::iequals(mf::utils::wide_to_bytes(*result), value);
    }
    return false;
}

bool api::GetIfExists(
        const boost::property_tree::wptree & pt,
        std::string property_path,
        std::string * to_set
    )
{
    const std::wstring property_path_w = mf::utils::bytes_to_wide(property_path);

    boost::optional<std::wstring> result =
        pt.get_optional<std::wstring>(property_path_w);
    if ( result && ! result->empty() )
    {
        *to_set = mf::utils::wide_to_bytes(*result);
        return true;
    }
    return false;
}

bool api::GetIfExists(
        const boost::property_tree::wptree & pt,
        std::string property_path,
        boost::posix_time::ptime * to_set
    )
{
    const std::wstring property_path_w = mf::utils::bytes_to_wide(property_path);

    boost::optional<std::wstring> result =
        pt.get_optional<std::wstring>(property_path_w);
    if ( result && ! result->empty() )
    {
        try {
            *to_set = boost::posix_time::time_from_string(
                mf::utils::wide_to_bytes(*result));
            if (*to_set != boost::date_time::not_a_date_time)
            {
                // Server returns time in CST - s23283
                typedef boost::date_time::local_adjustor<
                    boost::posix_time::ptime,
                    -6,
                    boost::posix_time::us_dst>
                        us_central;

                *to_set = us_central::local_to_utc(*to_set);

                return true;
            }
            // else fallthrough
        }
        catch (std::out_of_range const& e) {}
        catch (boost::bad_lexical_cast const& e) {}

        // Server returns time in CST, but hopefully not forever
        try {
            *to_set = boost::posix_time::from_iso_string(
                mf::utils::wide_to_bytes(*result));
            if (*to_set != boost::date_time::not_a_date_time)
                return true;
            // else fallthrough
        }
        catch (std::out_of_range const& e) {}
        catch (boost::bad_lexical_cast const& e) {}

    }

    *to_set = boost::date_time::not_a_date_time;
    return false;
}

bool api::GetIfExistsArrayFront(
        const boost::property_tree::wptree & pt,
        std::string property_path,
        std::string * to_set
    )
{
    std::wstring result;
    if ( GetIfExistsArrayFront(pt, property_path, &result) )
    {
        *to_set = mf::utils::wide_to_bytes(result);
        return true;
    }
    return false;
}

template <>
bool api::GetValueIfExists<std::string>(
        const boost::property_tree::wptree & pt,
        std::string * to_set
    )
{
    boost::optional<std::wstring> result = pt.get_value_optional<std::wstring>();
    if ( result && ! result->empty() )
    {
        *to_set = mf::utils::wide_to_bytes(*result);
        return true;
    }
    return false;
}

bool api::PropertyHasValue(
        const boost::property_tree::wptree & pt,
        std::string property_path,
        std::string value
    )
{
    const std::wstring value_wstr = mf::utils::bytes_to_wide(value);

    return PropertyHasValue(pt, property_path, value_wstr);
}
