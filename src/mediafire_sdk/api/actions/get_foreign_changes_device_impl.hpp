namespace mf
{
namespace api
{

template <typename TDeviceGetForeignChangesRequest>
GetForeignChangesDevice<TDeviceGetForeignChangesRequest>::
        DeviceGetForeignChangesErrorType::DeviceGetForeignChangesErrorType(
                const std::string & contact_key,
                uint32_t start_revision,
                const std::error_code & error_code,
                const std::string & error_string)
        : contact_key(contact_key),
          start_revision(start_revision),
          error_code(error_code),
          error_string(error_string)
{
}

template <typename TDeviceGetForeignChangesRequest>
std::shared_ptr<GetForeignChangesDevice<TDeviceGetForeignChangesRequest>>
GetForeignChangesDevice<TDeviceGetForeignChangesRequest>::Create(
        SessionMaintainer * stm,
        const std::string & contact_key,

        uint32_t latest_known_revision,
        CallbackType && callback)
{
    return std::shared_ptr<GetForeignChangesDevice>(new GetForeignChangesDevice(
            stm, contact_key, latest_known_revision, std::move(callback)));
}

template <typename TDeviceGetForeignChangesRequest>
void GetForeignChangesDevice<TDeviceGetForeignChangesRequest>::operator()()
{
    coro_();
}

template <typename TDeviceGetForeignChangesRequest>
GetForeignChangesDevice<TDeviceGetForeignChangesRequest>::
        GetForeignChangesDevice(SessionMaintainer * stm,
                                const std::string & contact_key,
                                uint32_t latest_known_revision,
                                CallbackType && callback)
        : stm_(stm),
          contact_key_(contact_key),
          latest_known_revision_(latest_known_revision),
          callback_(callback)
{
}
}  // namespace mf
}  // namespace api