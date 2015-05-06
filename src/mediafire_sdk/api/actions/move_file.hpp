#pragma once

#include "mediafire_sdk/api/session_maintainer.hpp"

#include "coroutine.hpp"

#include "boost/coroutine/all.hpp"

namespace mf
{
namespace api
{

template <typename TRequest>
class MoveFile : public Coroutine
{
public:
    // Some convenience typedefs
    using RequestType = TRequest;
    using ResponseType = typename RequestType::ResponseType;

    // The struct for the errors we might return
    struct ErrorType
    {
        ErrorType() {}

        ErrorType(const std::error_code & error_code,
                  boost::optional<std::string> error_string);

        std::error_code error_code;
        boost::optional<std::string> error_string;
    };

    using CallbackType
            = std::function<void(const ResponseType & response,
                                 const std::vector<ErrorType> & errors)>;

public:
    /**
     *  @brief Create an instance and get us the shared pointer to the created
     *instance.
     *
     *  @return std::shared_ptr Shared pointer to the created instance.
     **/
    static std::shared_ptr<MoveFile> Create(
            SessionMaintainer * stm,
            const std::string & quick_key,
            const std::string & destination_parent_folder_key,
            CallbackType && callback);

    void Cancel() override;

private:
    /**
     *  @brief  Private constructor.
     **/
    MoveFile(SessionMaintainer * stm,
             const std::string & quick_key,
             const std::string & destination_parent_folder_key,
             CallbackType && callback);

    void CoroutineBody(pull_type & yield) override;

private:
    SessionMaintainer * stm_;

    std::string quick_key_;

    std::string destination_parent_folder_key_;

    CallbackType callback_;

    ResponseType response_;
    std::vector<ErrorType> errors_;

    bool cancelled_ = false;

    SessionMaintainer::Request request_ = nullptr;
};

}  // namespace mf
}  // namespace api

#include "move_file_impl.hpp"