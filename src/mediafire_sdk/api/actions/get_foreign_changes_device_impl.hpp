namespace mf
{
namespace api
{

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetForeignChangesRequest>
GetForeignChangesDevice<TDeviceGetStatusRequest,
                        TDeviceGetForeignChangesRequest>::
        DeviceGetStatusErrorType::DeviceGetStatusErrorType(
                const std::error_code & error_code,
                const std::string & error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetForeignChangesRequest>
GetForeignChangesDevice<TDeviceGetStatusRequest,
                        TDeviceGetForeignChangesRequest>::
        DeviceGetForeignChangesErrorType::DeviceGetForeignChangesErrorType(
                uint32_t start_revision,
                const std::string & contact_key,
                const std::error_code & error_code,
                const std::string & error_string)
        : start_revision(start_revision),
          error_code(error_code),
          error_string(error_string)
{
}

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetForeignChangesRequest>
std::shared_ptr<GetForeignChangesDevice<TDeviceGetStatusRequest,
                                        TDeviceGetForeignChangesRequest>>
GetForeignChangesDevice<TDeviceGetStatusRequest,
                        TDeviceGetForeignChangesRequest>::
        Create(SessionMaintainer * stm,
               uint32_t latest_known_revision,
               const std::string & contact_key,
               CallbackType && callback)
{
    return std::shared_ptr<GetForeignChangesDevice>(new GetForeignChangesDevice(
            stm, latest_known_revision, contact_key, std::move(callback)));
}

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetForeignChangesRequest>
void GetForeignChangesDevice<TDeviceGetStatusRequest,
                             TDeviceGetForeignChangesRequest>::
operator()()
{
    coro_();
}

template <typename TDeviceGetStatusRequest,
          typename TDeviceGetForeignChangesRequest>
GetForeignChangesDevice<TDeviceGetStatusRequest,
                        TDeviceGetForeignChangesRequest>::
        GetForeignChangesDevice(SessionMaintainer * stm,
                                uint32_t latest_known_revision,
                                const std::string & contact_key,
                                CallbackType && callback)
        : stm_(stm),
          latest_known_revision_(latest_known_revision),
          contact_key_(contact_key),
          callback_(callback)
{
}
}  // namespace mf
}  // namespace api