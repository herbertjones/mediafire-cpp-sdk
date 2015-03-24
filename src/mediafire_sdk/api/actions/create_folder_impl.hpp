namespace mf
{
namespace api
{

template <class TRequest>
CreateFolder<TRequest>::ErrorType::ErrorType(
        const std::error_code & error_code,
        boost::optional<std::string> error_string)
        : error_code(error_code), error_string(error_string)
{
}

template <class TRequest>
std::shared_ptr<CreateFolder<TRequest>> CreateFolder<TRequest>::Create(
        SessionMaintainer * stm,
        const std::string & folder_name,
        CallbackType && callback)
{
    return std::shared_ptr<CreateFolder>(
            new CreateFolder(stm, folder_name, std::move(callback)));
}

template <class TRequest>
void CreateFolder<TRequest>::operator()()
{
    coro_();
}

template <class TRequest>
CreateFolder<TRequest>::CreateFolder(SessionMaintainer * stm,
                                     const std::string & folder_name,
                                     CallbackType && callback)
        : stm_(stm), folder_name_(folder_name), callback_(callback)
{
}
}  // namespace mf
}  // namespace api