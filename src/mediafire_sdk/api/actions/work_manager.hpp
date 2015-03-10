#pragma once

#include "boost/asio.hpp"

namespace mf
{
namespace api
{

template <class WorkType>
class WorkManager
{
public:
    WorkManager(boost::asio::io_service * io_service) : io_service_(io_service) {};

    void QueueWork(WorkType work)
    {
        work_queue_.push_back(work);

        PostMoreWork();
    }

    bool hasWorkRemaining()
    {
        if (num_work_in_io_service_ > 0)
            return true;
        else
            return false;
    }

    void SetMaxConcurrentWork(int max_work) { max_concurrent_work_ = max_work; }

private:
    // Posts more work onto io_service if needed
    void PostMoreWork()
    {
        if (num_work_in_io_service_ < max_concurrent_work_ && !work_queue_.empty())
        {
            ++num_work_in_io_service_;

            auto work = work_queue_.front();
            work_queue_.pop_front();

            io_service_->post([work, this](){ work->operator()(); --num_work_in_io_service_; PostMoreWork(); });
        }
    }

private:
    boost::asio::io_service * io_service_;

    std::deque<WorkType> work_queue_; // Queue of work to be posted onto io_service

    int num_work_in_io_service_ = 0;

    int max_concurrent_work_ = 10; // Default to 10
};

} // namespace mf
} // namespace api