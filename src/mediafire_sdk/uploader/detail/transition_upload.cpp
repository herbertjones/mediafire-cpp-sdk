/**
 * @file transition_upload.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "transition_upload.hpp"

namespace mf {
namespace uploader {
namespace detail {
namespace upload_transition {

std::string AsDateTime(std::time_t datetime)
{
    boost::posix_time::ptime ptime_(boost::posix_time::from_time_t(datetime));
    return boost::posix_time::to_iso_extended_string(ptime_) + "Z";
}

std::string AssembleQuery(
        const std::map< std::string, std::string > & query_map
    )
{
    namespace get_parameter = mf::utils::url::get_parameter;

    std::vector<std::string> query_pieces;
    query_pieces.reserve( query_map.size() );
    for ( const auto & pair : query_map )
    {
        if ( pair.second.empty() )
        {
            query_pieces.push_back(get_parameter::Encode(pair.first));
        }
        else
        {
            std::string piece = (pair.first);
            piece += "=";
            piece += get_parameter::Encode(pair.second);
            query_pieces.push_back(piece);
        }
    }

    std::string url = "?";
    url += boost::join(query_pieces, "&");
    return url;
}

}  // namespace upload_transition
}  // namespace detail
}  // namespace uploader
}  // namespace mf
