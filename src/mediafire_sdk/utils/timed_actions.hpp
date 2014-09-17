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

#include "boost/asio.hpp"
#include "boost/asio/steady_timer.hpp"
#include "boost/bind.hpp"
#include "boost/date_time/posix_time/posix_time_types.hpp"

#include "mediafire_sdk/utils/mutex.hpp"

namespace mf {
namespace utils {

/**
 * @class TimedActions
 * @brief A timer wrapper for when multiple items of same type need to be
 *        handled at individual timeouts.
 *
 * This class uses a single asio timer for multiple items instead of the nieve
 * approach of having one timer for each timeout.
 */
template<typename T>
class TimedActions :
    public std::enable_shared_from_this<TimedActions<T>>
{
public:
    /** The function definition of the callback used when actions timeout. */
    typedef std::function<void(T)> Callback;
    typedef std::shared_ptr<TimedActions> Pointer;

    /**
     * @param[in] io_service IO service used to execute acions.
     * @param[in] callback Function callback where actions are passed.
     *
     * @warning The life of the actions can outlive your usage of TimedActions.
     * If you bind a pointer to your callback the callback may be called after
     * the pointer object is destroyed.  To prevent this, you must call release
     * before destroying the object pointer.
     */
    static Pointer Create(
            boost::asio::io_service * io_service,
            Callback callback
        )
    {
        return std::shared_ptr<TimedActions>( new TimedActions(
                io_service, callback));
    }

    /**
     * @brief Stop performing events so the object can be destroyed
     * automatically once there are no longer any events in the io_service.
     */
    void Release()
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        running_ = false;
        timeout_timer_.cancel();
    }

    /**
     * @brief Remove all in progress timeouts.
     */
    void Clear()
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        data_.clear();
        timeout_timer_.cancel();
    }

    /**
     * @brief Add an action to be processed at the specified time.
     *
     * @param[in] duration Interval from now to execute action.
     * @param[in] action Action to pass to callback later.
     */
    void Add(
            std::chrono::milliseconds duration,
            const T & action
        )
    {
        const auto now = std::chrono::steady_clock::now();

        Add( now+duration, std::move(action) );
    }

    /**
     * @brief Add an action to be processed at the specified time.
     *
     * @param[in] timeout_time Time for action to execute.
     * @param[in] action Action to pass to callback later.
     */
    void Add(
            std::chrono::time_point<std::chrono::steady_clock> timeout_time,
            const T & action
        )
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        data_.insert( std::make_pair(timeout_time, std::move(action)) );

        io_service_->post(
            boost::bind( &TimedActions<T>::ResetTimer, this->shared_from_this() ));
    }

    /**
     * @brief Get the number of actions awaiting processing.
     *
     * @return Number of actions yet to be processed.
     */
    std::size_t Size()
    {
        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);
        return data_.size();
    }

private:
    boost::asio::io_service * io_service_;
    boost::asio::steady_timer timeout_timer_;

    bool running_;

    Callback callback_;
    std::multimap<std::chrono::time_point<std::chrono::steady_clock>, T> data_;

    mf::utils::mutex mutex_;

    /**
     * @param[in] io_service IO service used to execute acions.
     * @param[in] callback Function callback where actions are passed.
     */
    TimedActions(
            boost::asio::io_service * io_service,
            Callback callback
        ) :
        io_service_(io_service),
        timeout_timer_(*io_service),
        running_(true),
        callback_(callback)
    {
    }

    // Only call on io_service
    void ResetTimer()
    {
        const auto now = std::chrono::steady_clock::now();

        mf::utils::lock_guard<mf::utils::mutex> lock(mutex_);

        while (running_ && !data_.empty())
        {
            const auto next = data_.begin();
            const auto timeout = next->first;
            if ( timeout <= now )
            {
                callback_(next->second);
                data_.erase(next);
            }
            else
            {
                timeout_timer_.expires_at(timeout);

                timeout_timer_.async_wait(
                    boost::bind(
                        &TimedActions<T>::HandleTimeout,
                        this->shared_from_this(),
                        boost::asio::placeholders::error
                    )
                );
                break;
            }
        }
    }

    // Only call on io_service
    void HandleTimeout(
            const boost::system::error_code & err
        )
    {
        if (!err)
        {
            ResetTimer();
        }
    }
};

}  // namespace utils
}  // namespace mf
