namespace mf
{
namespace api
{

template <class TRequest>
GetInfoFile<TRequest>::ErrorType::ErrorType(
        const std::string & folder_key,
        const std::error_code & error_code,
        const std::string & error_string)
        : folder_key(folder_key),
          error_code(error_code),
          error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<GetInfoFile<TRequest>> GetInfoFile<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & folder_key,
        CallbackType && callback)
{
    return std::shared_ptr<GetInfoFile>(
            new GetInfoFile(stm, folder_key, std::move(callback)));
}

template <class TRequest>
void GetInfoFile<TRequest>::operator()()
{
    coro_();
}

template <class TRequest>
GetInfoFile<TRequest>::GetInfoFile(SessionMaintainer * stm,
                                       const std::string & folder_key,
                                       CallbackType && callback)
        : stm_(stm), folder_key_(folder_key), callback_(callback)
{
}
}  // namespace mf
}  // namespace api