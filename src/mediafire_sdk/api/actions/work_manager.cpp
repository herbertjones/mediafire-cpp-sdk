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
    work_queue_.push_back(WorkYieldPair(coroutine, yield));
}

void WorkManager::SetMaxConcurrentWork(int max_work)
{
    max_concurrent_work_ = max_work;
}

void WorkManager::ExecuteWork()
{
    auto self = shared_from_this();

    std::vector<boost::coroutines::coroutine<void>::pull_type *> yield_vector;
    while (num_work_in_io_service_ < max_concurrent_work_
           && !work_queue_.empty())
    {
        ++num_work_in_io_service_;

        auto work_yield_pair = work_queue_.front();
        work_queue_.pop_front();

        // Post work onto io_service
        auto work = work_yield_pair.first;
        io_service_->post([work, this, self]()
                          {
                              work->operator()();
                              --num_work_in_io_service_;
                          });

        // Enqueue the yield so we can yield after posting as much work as
        // possible
        yield_vector.push_back(work_yield_pair.second);
    }

    for (auto & yield : yield_vector)
    {
        if (yield != nullptr)
        {
            yield->operator()();
            ExecuteWork();  // Post more work if available
        }
    }
}

}  // namespace mf
}  // namespace api