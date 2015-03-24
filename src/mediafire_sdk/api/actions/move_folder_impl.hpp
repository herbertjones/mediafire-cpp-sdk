namespace mf
{
namespace api
{

template <class TRequest>
MoveFolder<TRequest>::ErrorType::ErrorType(
        const std::error_code & error_code,
        boost::optional<std::string> error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<MoveFolder<TRequest>> MoveFolder<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & folder_key,
        const std::string & destination_parent_folder_key,
        CallbackType && callback)
{
    return std::shared_ptr<MoveFolder>(
            new MoveFolder(stm,
                           folder_key,
                           destination_parent_folder_key,
                           std::move(callback)));
}

template <class TRequest>
void MoveFolder<TRequest>::operator()()
{
    coro_();
}

template <class TRequest>
MoveFolder<TRequest>::MoveFolder(
        SessionMaintainer * stm,
        const std::string & folder_key,
        const std::string & destination_parent_folder_key,
        CallbackType && callback)
        : stm_(stm),
          folder_key_(folder_key),
          destination_parent_folder_key_(destination_parent_folder_key),
          callback_(callback)
{
}
}  // namespace mf
}  // namespace api