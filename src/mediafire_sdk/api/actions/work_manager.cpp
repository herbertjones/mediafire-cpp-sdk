#include "work_manager.hpp"

namespace mf
{
namespace api
{

std::shared_ptr<WorkManager> WorkManager::Create(
        boost::asio::io_service * io_service)
{
    return std::shared_ptr<WorkManager>(new WorkManager(io_service));
}

void WorkManager::QueueWork(
        std::vector<std::shared_ptr<Coroutine>> coroutines,
        boost::coroutines::coroutine<void>::pull_type * yield)
{
    for (auto & coroutine : coroutines)
    {
        QueueWork(coroutine, yield);
    }
}

void WorkManager::QueueWork(
        std::shared_ptr<Coroutine> coroutine,
        boost::coroutines::coroutine<void>::pull_type * yield)
{
    work_queue_.push(WorkYieldPair(coroutine, yield));
}

void WorkManager::SetMaxConcurrentWork(int max_work)
{
    max_concurrent_work_ = max_work;
}

void WorkManager::ExecuteWork()
{
    auto self = shared_from_this();

    std::queue<boost::coroutines::coroutine<void>::pull_type *> yield_queue;

    while (true)
    {
        bool queue_more_work
                = false;  // Flag to keep track of whether or not to queue more
        // work. This is needed because if we were to call
        // ExecuteWork recursively, we would blow up the stack
        // if we have too much work to queue.

        while (in_progress_list_.size() < max_concurrent_work_
               && !work_queue_.empty())
        {
            auto work_yield_pair = work_queue_.front();
            work_queue_.pop();

            auto work = work_yield_pair.first;
            auto yield = work_yield_pair.second;

            CoroutineWorkManagerAttorney::SetCompletionHandler(
                    work,
                    [this, self, work]()
                    {
                        in_progress_list_.remove(work);
                    });

            // Post work onto io_service
            io_service_->post([work]()
                              {
                                  work->Start();
                              });

            // Enqueue the yield so we can yield after posting as much work as
            // possible
            yield_queue.push(yield);

            queue_more_work = true;
        }

        if (!yield_queue.empty())
        {
            auto yield = yield_queue.front();
            yield_queue.pop();

            if (yield != nullptr)
            {
                yield->operator()();
            }
        }

        if (yield_queue.empty() && !queue_more_work)
            break;
    }
}

void WorkManager::Cancel()
{
    auto self = shared_from_this();

    io_service_->post([this, self]()
                    {
                        while (!work_queue_.empty())
                        {
                            auto work_yield_pair = work_queue_.front();
                            work_queue_.pop();

                            auto work = work_yield_pair.first;

                            work->Cancel();
                        }

                        for (auto & work : in_progress_list_)
                        {
                            work->Cancel();
                        }
                        in_progress_list_.clear();
                    });
}

}  // namespace mf
}  // namespace api