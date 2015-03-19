namespace mf
{
namespace api
{

template <typename TRequest>
CloneCloudTree<TRequest>::CloneCloudTree(
        SessionMaintainer * stm,
        const std::string & folder_key,
        std::shared_ptr<WorkManager> work_manager,
        CallbackType && callback)
        : stm_(stm), work_manager_(work_manager), callback_(std::move(callback))
{
    new_folder_keys_.push_back(folder_key);
}

template <typename TRequest>
std::shared_ptr<CloneCloudTree<TRequest>> CloneCloudTree<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & folder_key,
        std::shared_ptr<WorkManager> work_manager,
        CallbackType && callback)
{
    return std::shared_ptr<CloneCloudTree>(new CloneCloudTree(
            stm, folder_key, work_manager, std::move(callback)));
}

template <typename TRequest>
void CloneCloudTree<TRequest>::operator()()
{
    coro_();
}

}  // namespace api
}  // namespace mf