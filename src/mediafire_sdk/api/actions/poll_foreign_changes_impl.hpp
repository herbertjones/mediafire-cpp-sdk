namespace mf
{

namespace api
{
template <typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
std::shared_ptr<PollForeignChanges<TDeviceGetForeignChangesRequest,
                                   TFolderGetInfoRequest,
                                   TFileGetInfoRequest>>
PollForeignChanges<TDeviceGetForeignChangesRequest,
                   TFolderGetInfoRequest,
                   TFileGetInfoRequest>::Create(SessionMaintainer * stm,
                                                const std::string & contact_key,
                                                uint32_t revision,
                                                std::shared_ptr<WorkManager>
                                                        work_manager,
                                                CallbackType && callback)
{
    return std::shared_ptr<PollForeignChanges>(new PollForeignChanges(
            stm, contact_key, revision, work_manager, std::move(callback)));
}

template <typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
void PollForeignChanges<TDeviceGetForeignChangesRequest,
                        TFolderGetInfoRequest,
                        TFileGetInfoRequest>::
operator()()
{
    coro_();
}

template <typename TDeviceGetForeignChangesRequest,
          typename TFolderGetInfoRequest,
          typename TFileGetInfoRequest>
PollForeignChanges<TDeviceGetForeignChangesRequest,
                   TFolderGetInfoRequest,
                   TFileGetInfoRequest>::
        PollForeignChanges(SessionMaintainer * stm,
                           const std::string & contact_key,
                           uint32_t revision,
                           std::shared_ptr<WorkManager> work_manager,
                           CallbackType && callback)
        : stm_(stm),
          contact_key_(contact_key),
          revision_(revision),
          work_manager_(work_manager),
          callback_(callback)
{
}

}  // namespace mf
}  // namespace api