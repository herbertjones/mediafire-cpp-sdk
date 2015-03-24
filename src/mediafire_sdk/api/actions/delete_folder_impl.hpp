namespace mf
{
namespace api
{

template <class TRequest>
DeleteFolder<TRequest>::ErrorType::ErrorType(
        const std::error_code & error_code,
        boost::optional<std::string> error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<DeleteFolder<TRequest>> DeleteFolder<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & folder_key,
        CallbackType && callback)
{
    return std::shared_ptr<DeleteFolder>(
            new DeleteFolder(stm,
                           folder_key,
                           std::move(callback)));
}

template <class TRequest>
void DeleteFolder<TRequest>::operator()()
{
    coro_();
}

template <class TRequest>
DeleteFolder<TRequest>::DeleteFolder(
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