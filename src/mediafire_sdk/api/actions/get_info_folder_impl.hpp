namespace mf
{
namespace api
{

template <class TRequest>
GetInfoFolder<TRequest>::ErrorType::ErrorType(
        const std::string & folder_key,
        const std::error_code & error_code,
        const std::string & error_string)
        : folder_key(folder_key),
          error_code(error_code),
          error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<GetInfoFolder<TRequest>>
GetInfoFolder<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & folder_key,
        CallbackType && callback)
{
    return std::shared_ptr<GetInfoFolder>(new GetInfoFolder(
            stm, folder_key, std::move(callback)));
}

template <class TRequest>
void GetInfoFolder<TRequest>::operator()()
{
    coro_();
}

template <class TRequest>
GetInfoFolder<TRequest>::GetInfoFolder(
        SessionMaintainer * stm,
        const std::string & folder_key,
        CallbackType && callback)
        : stm_(stm),
          folder_key_(folder_key),
          callback_(callback)
{
}
}  // namespace mf
}  // namespace api