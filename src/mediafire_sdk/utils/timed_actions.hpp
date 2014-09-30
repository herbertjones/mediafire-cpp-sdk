/**
 * @file timed_actions.hpp
 * @author Herbert Jones
 * @brief A class for encapsulating a series of timed actions effeciently.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <memory>

#include "mediafire_sdk/utils/timed_events.hpp"

namespace mf {
namespace utils {

/**
 * @class TimedActions
 * @brief A timer wrapper for when multiple items of same type need to be
 *        handled at individual timeouts.
 *
 * This class uses a single asio timer for multiple actions instead of the naive
 * approach of having one timer for each timeout.
 *
 * @warning When cancelling, or when TimedActions is destroyed, all unfired
 * events will fire with std::errc::operation_aborted.  The events will still be
 * processed by the io_service passed to Create.  If the actions may now hold
 * invalid pointers, so if ignoring operation_aborted, be sure to use a shared
 * pointer.
 */
class TimedActions :
    public std::enable_shared_from_this<TimedActions>
{
public:
    using Action = std::function<void(const std::error_code &)>;
    using Pointer = std::shared_ptr<TimedActions>;
    using Impl = TimedEvents<Action>;
    using TimePoint = Impl::TimePoint;

    /**
     * @param[in] io_service IO service used to execute acions.
     * @param[in] callback Function callback where actions are passed.
     */
    static Pointer Create(
            boost::asio::io_service * io_service
        )
    {
        return std::shared_ptr<TimedActions>(new TimedActions(io_service));
    }

    /**
     * @brief Stop performing events so the object can be destroyed
     * automatically once there are no longer any events in the io_service.
     */
    void Stop()
    {
        impl_->Stop();
    }

    /**
     * @brief Cancel all in progress events.
     */
    void CancelAll()
    {
        impl_->CancelAll();
    }

    /**
     * @brief Add an action to be processed at the specified time.
     *
     * @param[in] duration Interval from now to execute action.
     * @param[in] action Action to execute later.
     */
    void Add(
            std::chrono::milliseconds duration,
            const Action & action
        )
    {
        impl_->Add(duration, action);
    }

    /**
     * @brief Add an action to be processed at the specified time.
     *
     * @param[in] timeout_time Time for action to execute.
     * @param[in] action Action to execute later.
     */
    void Add(
            TimePoint timeout_time,
            const Action & action
        )
    {
        impl_->Add(timeout_time, action);
    }

    /**
     * @brief Get the number of actions awaiting processing.
     *
     * @return Number of actions yet to be processed.
     */
    std::size_t Size()
    {
        return impl_->Size();
    }

private:
    Impl::Pointer impl_;

    TimedActions(
            boost::asio::io_service * io_service
        ) :
        impl_(Impl::Create(io_service,
                [](Action action, const std::error_code & ec) { action(ec);}))
    {
    }
};

}  // namespace utils
}  // namespace mf
