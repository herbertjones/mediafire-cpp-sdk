namespace mf
{
namespace api
{

template <class TRequest>
DeleteFile<TRequest>::ErrorType::ErrorType(
        const std::error_code & error_code,
        boost::optional<std::string> error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<DeleteFile<TRequest>> DeleteFile<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & quick_key,
        CallbackType && callback)
{
    return std::shared_ptr<DeleteFile>(
            new DeleteFile(stm, quick_key, std::move(callback)));
}

template <class TRequest>
void DeleteFile<TRequest>::operator()()
{
    coro_();
}

template <class TRequest>
DeleteFile<TRequest>::DeleteFile(SessionMaintainer * stm,
                                     const std::string & quick_key,
                                     CallbackType && callback)
        : stm_(stm), quick_key_(quick_key), callback_(callback)
{
}
}  // namespace mf
}  // namespace api