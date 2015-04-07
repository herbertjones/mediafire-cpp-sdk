namespace mf
{
namespace api
{

template <typename TDeviceGetForeignResourcesRequest>
GetForeignResourcesDevice<TDeviceGetForeignResourcesRequest>::
        ErrorType::ErrorType(
                const std::error_code & error_code,
                const boost::optional<std::string> & error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <typename TDeviceGetForeignResourcesRequest>
std::shared_ptr<GetForeignResourcesDevice<TDeviceGetForeignResourcesRequest>>
GetForeignResourcesDevice<TDeviceGetForeignResourcesRequest>::Create(
        SessionMaintainer * stm,
        CallbackType && callback)
{
    return std::shared_ptr<GetForeignResourcesDevice>(
            new GetForeignResourcesDevice(stm, std::move(callback)));
}

template <typename TDeviceGetForeignResourcesRequest>
void GetForeignResourcesDevice<TDeviceGetForeignResourcesRequest>::operator()()
{
    coro_();
}

template <typename TDeviceGetForeignResourcesRequest>
GetForeignResourcesDevice<TDeviceGetForeignResourcesRequest>::
        GetForeignResourcesDevice(SessionMaintainer * stm,
                                  CallbackType && callback)
        : stm_(stm), callback_(callback)
{
}
}  // namespace mf
}  // namespace api