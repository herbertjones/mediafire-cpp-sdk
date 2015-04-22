#pragma once

#include <queue>
#include <memory>

#include "boost/asio.hpp"
#include "boost/coroutine/all.hpp"

#include "coroutine.hpp"

namespace mf
{
namespace api
{

class WorkManager : public std::enable_shared_from_this<WorkManager>
{
public:
    /**
     *  @brief Creates shared_ptr to a newly created instance of this class.
     */
    static std::shared_ptr<WorkManager> Create(
            boost::asio::io_service * io_service);

    /**
     *  @brief Queue some work (coroutines) along with the corresponding yield
     * from the parent coroutine. This does not yield to allow for queueing of
     * more work.
     */
    void QueueWork(std::vector<std::shared_ptr<Coroutine>> coroutines,
                   boost::coroutines::coroutine<void>::pull_type * yield);

    /**
     *  @brief Queue a piece of work (Coroutine) along with the corresponding
     * yield from the parent coroutine. This does not yield to allow for
     * queueing of
     * more work.
     */
    void QueueWork(std::shared_ptr<Coroutine> coroutine,
                   boost::coroutines::coroutine<void>::pull_type * yield);

    /**
     *  @brief Maximum amount of work that can be queued by us onto the
     * io_service.
     */
    void SetMaxConcurrentWork(int max_work);

    /**
     *  @brief Dequeue and posts some work onto the io_sevice_. This then
     *yields the amount of work queued onto the io_service.
     */
    void ExecuteWork();

    /**
     *  @brief Cancel everything on the work_queue_
     */
    void Cancel();

private:
    WorkManager(boost::asio::io_service * io_service)
            : io_service_(io_service){};

private:
    using WorkYieldPair
            = std::pair<std::shared_ptr<Coroutine>,
                        boost::coroutines::coroutine<void>::pull_type *>;

    boost::asio::io_service * io_service_;

    std::queue<WorkYieldPair>
            work_queue_;  // Queue of work to be posted onto io_service

    int num_work_in_progress_ = 0;

    int max_concurrent_work_ = 10;  // Default to 10
};

}  // namespace mf
}  // namespace api