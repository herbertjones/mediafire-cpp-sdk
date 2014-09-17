/**
 * @file bandwidth_analyser.hpp
 * @author Herbert Jones
 * @brief Class to keep track of bandwidth.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "boost/asio.hpp"

#include <chrono>

namespace mf {
namespace http {

/**
 * @class BandwidthAnalyser
 * @brief Records bandwidth.
 */
class BandwidthAnalyser
{
public:
    /**
     * @brief Create BandwidthAnalyser.
     *
     * @param[in] ios IO service used internally.
     */
    BandwidthAnalyser(
            boost::asio::io_service * ios
        );

    /**
     * @brief Record the number of incoming bytes.
     *
     * @param[in] bytes Total bytes for range.
     * @param[in] start_time Start of range.
     * @param[in] end_time End of range.
     */
    void RecordIncomingBytes(
            size_t bytes,
            std::chrono::time_point<std::chrono::steady_clock> start_time,
            std::chrono::time_point<std::chrono::steady_clock> end_time
        );

    /**
     * @brief Record the number of outgoing bytes.
     *
     * @param[in] bytes Total bytes for range.
     * @param[in] start_time Start of range.
     * @param[in] end_time End of range.
     */
    void RecordOutgoingBytes(
            size_t bytes,
            std::chrono::time_point<std::chrono::steady_clock> start_time,
            std::chrono::time_point<std::chrono::steady_clock> end_time
        );

private:
    boost::asio::io_service::strand event_strand_;

    enum ChangeType {
        kIncoming,
        kOutgoing,
    };

    void ProcessChange(
            ChangeType,
            size_t bytes,
            std::chrono::time_point<std::chrono::steady_clock> start_time,
            std::chrono::time_point<std::chrono::steady_clock> end_time
        );
};

}  // namespace http
}  // namespace mf
