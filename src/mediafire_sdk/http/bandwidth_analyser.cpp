/**
 * @file bandwidth_analyser.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "bandwidth_analyser.hpp"

#include <functional>

namespace hl = mf::http;
namespace asio = boost::asio;

hl::BandwidthAnalyser::BandwidthAnalyser(
        asio::io_service * ios
    ) :
    event_strand_(*ios)
{
}

void hl::BandwidthAnalyser::RecordIncomingBytes(
        const size_t bytes,
        std::chrono::time_point<std::chrono::steady_clock> start_time,
        std::chrono::time_point<std::chrono::steady_clock> end_time
    )
{
    // Do all work in a single thread.
    event_strand_.post(
            std::bind(
                &BandwidthAnalyser::ProcessChange, this,
                kIncoming,
                bytes,
                start_time,
                end_time
            )
        );
}

void hl::BandwidthAnalyser::RecordOutgoingBytes(
        const size_t bytes,
        std::chrono::time_point<std::chrono::steady_clock> start_time,
        std::chrono::time_point<std::chrono::steady_clock> end_time
    )
{
    // Do all work in a single thread.
    event_strand_.post(
            std::bind(
                &BandwidthAnalyser::ProcessChange, this,
                kOutgoing,
                bytes,
                start_time,
                end_time
            )
        );
}

void hl::BandwidthAnalyser::ProcessChange(
        ChangeType ct,
        size_t bytes,
        std::chrono::time_point<std::chrono::steady_clock> start_time,
        std::chrono::time_point<std::chrono::steady_clock> end_time
    )
{
    switch ( ct )
    {
        case kIncoming:
            break;
        case kOutgoing:
            break;
    }
}
