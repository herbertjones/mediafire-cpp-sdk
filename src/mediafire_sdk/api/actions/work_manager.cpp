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

        while (num_work_in_progress_ < max_concurrent_work_
               && !work_queue_.empty())
        {
            ++num_work_in_progress_;

            auto work_yield_pair = work_queue_.front();
            work_queue_.pop();

            // Post work onto io_service
            auto work = work_yield_pair.first;
            io_service_->post([work, this, self]()
                              {
                                  work->Start();
                              });

            // Enqueue the yield so we can yield after posting as much work as
            // possible
            yield_queue.push(work_yield_pair.second);

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

            --num_work_in_progress_;
        }

        if (yield_queue.empty() && !queue_more_work)
            break;
    }
}

}  // namespace mf
}  // namespace api