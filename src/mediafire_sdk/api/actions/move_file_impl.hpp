namespace mf
{
namespace api
{

template <class TRequest>
MoveFile<TRequest>::ErrorType::ErrorType(
        const std::error_code & error_code,
        boost::optional<std::string> error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<MoveFile<TRequest>> MoveFile<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & quick_key,
        const std::string & destination_parent_folder_key,
        CallbackType && callback)
{
    return std::shared_ptr<MoveFile>(new MoveFile(stm,
                                                  quick_key,
                                                  destination_parent_folder_key,
                                                  std::move(callback)));
}

template <class TRequest>
void MoveFile<TRequest>::operator()()
{
    coro_();
}

template <class TRequest>
MoveFile<TRequest>::MoveFile(SessionMaintainer * stm,
                             const std::string & quick_key,
                             const std::string & destination_parent_folder_key,
                             CallbackType && callback)
        : stm_(stm),
          quick_key_(quick_key),
          destination_parent_folder_key_(destination_parent_folder_key),
          callback_(callback)
{
}
}  // namespace mf
}  // namespace api