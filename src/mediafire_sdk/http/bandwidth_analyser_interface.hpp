/**
 * @file bandwidth_analyser_interface.hpp
 * @author Herbert Jones
 * @brief Class to keep track of bandwidth.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "boost/asio.hpp"

#include <chrono>
#include <memory>

namespace mf {
namespace http {

/**
 * @interface BandwidthAnalyserInterface
 * @brief Records bandwidth.
 */
class BandwidthAnalyserInterface
{
public:
    using Pointer = std::shared_ptr<BandwidthAnalyserInterface>;

    virtual ~BandwidthAnalyserInterface() = default;

    /**
     * @brief Record the number of incoming bytes.
     *
     * @param[in] bytes Total bytes for range.
     * @param[in] start_time Start of range.
     * @param[in] end_time End of range.
     */
    virtual void RecordIncomingBytes(
            size_t bytes,
            std::chrono::time_point<std::chrono::steady_clock> start_time,
            std::chrono::time_point<std::chrono::steady_clock> end_time
        ) = 0;

    /**
     * @brief Record the number of outgoing bytes.
     *
     * @param[in] bytes Total bytes for range.
     * @param[in] start_time Start of range.
     * @param[in] end_time End of range.
     */
    virtual void RecordOutgoingBytes(
            size_t bytes,
            std::chrono::time_point<std::chrono::steady_clock> start_time,
            std::chrono::time_point<std::chrono::steady_clock> end_time
        ) = 0;
};

}  // namespace http
}  // namespace mf
