/**
 * @file api/actions/detail/concurrent_coroutine.hpp
 * @author Herbert Jones
 * @brief Implementaion for coroutine that performs concurrent actions..
 * @copyright Copyright 2015 Mediafire
 */
#pragma once

#include "coroutine.hpp"

namespace mf
{
namespace api
{
namespace detail
{

/**
 * @class ConcurrentCoroutine
 * @brief Coroutine with convenience methods to make concurrent calls.
 */
template <typename T>
class ConcurrentCoroutine : public Coroutine<T>
{
public:
    /**
     * @brief Like CallApi, but only enqueues the action to be started later
     *        after calling WaitForEnqueued.
     *
     * See CallApi.
     *
     * @param[in] request The API request to pass to SessionMaintainer::Call.
     * @param[out] response The API response location where to store the
     *                      response of Call().
     */
    template <typename RequestType, typename ResponseType>
    void EnqueueApi(RequestType request, ResponseType & store)
    {
        auto self = this->shared_from_this();

        awaiting_actions_.push_back(
                [this, self, request, &store]()
                {
                    Coroutine<T>::stm_->Call(
                            std::move(request),
                            [this, self, &store](ResponseType response)
                            {
                                store = std::move(response);

                                FinishEnqueued();
                            });
                });
    }

    /**
     * @brief Like CallAction, but only enqueues the action to be started later
     *        after calling WaitForEnqueued.
     *
     * See CallAction.
     *
     * @param[out] request Action pointer to store the created action.
     * @param[in] args Arguments to pass to the Create method of the given
     *                 action.
     */
    template <typename CoroPtr, typename... Args>
    void EnqueueAction(std::vector<CoroPtr> & coro_ptrs, Args &&... args)
    {
        using Coro = typename CoroPtr::element_type;

        auto self = this->shared_from_this();

        auto coro = Coro::CreateUnstarted(Coroutine<T>::stm_,
                                          [self, this](ActionResult, CoroPtr)
                                          {
                                              FinishEnqueued();
                                          },
                                          std::forward<Args>(args)...);
        coro_ptrs.push_back(coro);

        awaiting_actions_.push_back([coro]()
                                    {
                                        // Start coroutine
                                        coro->Start();
                                    });
    }

    /**
     * @brief Wait for enqueued actions to complete.  Must call with yield.
     */
    void WaitForEnqueued()
    {
        while (!awaiting_actions_.empty()
               && in_progress_count_ < max_concurrent_)
        {
            // Start next.
            awaiting_actions_.front()();

            // Remove
            awaiting_actions_.pop_front();

            ++in_progress_count_;
        }
    }

    /**
     * @brief Set the number of max concurrent calls to be made.
     *
     * @param[in] max_concurrent New max concurrent calls.
     */
    void SetMaxConcurrent(uint32_t max_concurrent)
    {
        max_concurrent_ = max_concurrent;
    }

protected:
    ConcurrentCoroutine(mf::api::SessionMaintainer * stm,
                        typename Coroutine<T>::Callback callback,
                        uint32_t max_concurrent)
            : Coroutine<T>(stm, callback),
              max_concurrent_(max_concurrent),
              in_progress_count_(0)
    {
    }

private:
    friend class Coroutine<T>;

    void FinishEnqueued()
    {
        --in_progress_count_;

        if (awaiting_actions_.empty() && in_progress_count_ == 0)
        {
            // Resume coroutine.
            auto self = this->shared_from_this();
            (*self)();
        }
        else if (!awaiting_actions_.empty())
        {
            awaiting_actions_.front()();

            awaiting_actions_.pop_front();

            ++in_progress_count_;
        }
    }

protected:
    uint32_t max_concurrent_;

private:
    std::deque<std::function<void()>> awaiting_actions_;
    uint32_t in_progress_count_;
};

}  // namespace detail
}  // namespace api
}  // namespace mf
