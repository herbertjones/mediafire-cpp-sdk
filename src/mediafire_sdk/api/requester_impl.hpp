/**
 * @file requester_impl.hpp
 * @author Herbert Jones
 * @brief http request wrapper for API calls.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <map>
#include <memory>
#include <string>

#include "mediafire_sdk/api/config.hpp"
#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/utils/string.hpp"
#include "mediafire_sdk/http/http_request.hpp"
#include "mediafire_sdk/http/headers.hpp"
#include "mediafire_sdk/http/post_data_pipe_interface.hpp"

namespace mf {
namespace api {
namespace detail {

/** Create struct which is only well defined if X is member of passed in class.
 */
#define CREATE_HAS_MEMBER(X)                                                   \
template<typename T> class Has##X                                              \
{                                                                              \
    struct Fallback { int X; };                                                \
    struct Derived : T, Fallback { };                                          \
    /* blank */                                                                \
    template<typename U, U> struct Check;                                      \
    /* blank */                                                                \
    typedef char ArrayOfOne[1];                                                \
    typedef char ArrayOfTwo[2];                                                \
    /* blank */                                                                \
    template<typename U> static ArrayOfOne &                                   \
    func(Check<int Fallback::*, &U::X> *);                                     \
    template<typename U> static ArrayOfTwo & func(...);                        \
public:                                                                        \
    typedef Has##X type;                                                       \
    enum { value = sizeof(func<Derived>(0)) == 2 };                            \
};
/** HasGetPostData<T>::value is set if T has method GetPostData */
CREATE_HAS_MEMBER(GetPostData);
/** HasGetPostDataPipe<T>::value is set if T has method GetPostDataPipe */
CREATE_HAS_MEMBER(GetPostDataPipe);
#undef CREATE_HAS_MEMBER

template<typename ApiFunctor>
typename std::enable_if<HasGetPostData<ApiFunctor>::value, void>::type
HandlePost(
        mf::http::HttpRequest * request,
        ApiFunctor * api_functor
    )
{
    // Setup POST data
    mf::http::SharedBuffer::Pointer sb = api_functor->GetPostData();
    request->SetPostData(sb);
    request->SetHeader("Content-Type", "application/x-www-form-urlencoded");
}

template<typename ApiFunctor>
typename std::enable_if<HasGetPostDataPipe<ApiFunctor>::value, void>::type
HandlePost(
        mf::http::HttpRequest * request,
        ApiFunctor * api_functor
    )
{
    // POST data pipe override
    std::shared_ptr<mf::http::PostDataPipeInterface> pdpi =
        api_functor->GetPostDataPipe();
    request->SetPostDataPipe(pdpi);
    request->SetHeader("Content-Type", "application/x-www-form-urlencoded");
}

template<typename ApiFunctor>
typename std::enable_if< ! HasGetPostDataPipe<ApiFunctor>::value
    && ! HasGetPostData<ApiFunctor>::value , void>::type
HandlePost(
        mf::http::HttpRequest * /* request */,
        ApiFunctor * /* api_functor */
    )
{
    // Do nothing.
}

/**
 * Internal implementation for Requester.
 */
template<typename ApiFunctor, typename IoService>
class RequesterImpl :
    public std::enable_shared_from_this<RequesterImpl<ApiFunctor, IoService>>,
    public mf::http::RequestResponseInterface
{
public:
    RequesterImpl(
            mf::http::HttpConfig::ConstPointer http_config,
            ApiFunctor api_functor,
            typename ApiFunctor::CallbackType cb,
            IoService * callback_ios,
            const std::string & hostname
            ) :
        http_config_(http_config),
        api_functor_(std::move(api_functor)),
        cb_(std::move(cb)),
        callback_ios_(callback_ios),
        hostname_(hostname)
    {
        assert(callback_ios_);
    }

    virtual ~RequesterImpl() {}

    virtual void RedirectHeaderReceived(
            std::string /* raw_header */,
            std::map<std::string, std::string> /* headers */,
            mf::http::Url /* new_url */
        ) override
    {
    }

    virtual void ResponseHeaderReceived(
            mf::http::Headers headers
        ) override
    {
        headers_ = headers;
    }

    virtual void ResponseContentReceived(
            std::size_t /* start_pos */,
            std::shared_ptr<mf::http::BufferInterface> buffer
        ) override
    {
        content_.append( reinterpret_cast<const char*>(buffer->Data()),
            buffer->Size() );
    }

    virtual void RequestResponseErrorEvent(
            std::error_code error_code,
            std::string error_text
        ) override
    {
        auto self = this->shared_from_this();
        auto action([this, self, error_code, error_text]()
            {
                self->api_functor_.HandleError(
                        url_,
                        std::move(error_code),
                        error_text
                    );
            });
        callback_ios_->dispatch( action );

        // This will delete this as well once self is gone.
        request_.reset();
    }

    virtual void RequestResponseCompleteEvent() override
    {
        auto self = this->shared_from_this();

        if ( ! self->headers_ )
        {
            RequestResponseErrorEvent(
                    make_error_code( api::api_code::ContentInvalidData ),
                    "Content received with no headers."
                );
        }
        else
        {
            auto action([this, self]()
                {
                    self->api_functor_.HandleContent(
                        url_,
                        *headers_,
                        content_ );
                });
            callback_ios_->dispatch( action );
        }

        // This will delete this as well once self is gone.
        request_.reset();
    }

    /**
     * @brief Call to start the request
     */
    mf::http::HttpRequest::Pointer Init(RequestStarted start)
    {
        api_functor_.SetCallback(cb_);

        // Session token may be included here if session token GET type.
        url_ = api_functor_.Url(hostname_);

        request_ = mf::http::HttpRequest::Create(
                http_config_,
                std::static_pointer_cast<mf::http::RequestResponseInterface>(
                        this->shared_from_this()),
                url_ );

        // Session token may be included here if session token POST type.
        HandlePost<ApiFunctor>(request_.get(), &api_functor_);

        if (start == RequestStarted::Yes )
        {
            // Start the request.
            request_->Start();
        }

        return request_;
    }

    virtual void Cancel()
    {
        if (request_)
        {
            // This should cause RequestResponseErrorEvent to fire if
            // cancellation is accepted.  If cancel is too late to matter, then
            // RequestResponseCompleteEvent may fire.
            request_->Cancel();
        }
    }

    virtual void Fail(
            const std::error_code & error_code,
            const std::string & error_string
        )
    {
        if (request_)
        {
            // This should cause RequestResponseErrorEvent to fire if
            // cancellation is accepted.  If cancel is too late to matter, then
            // RequestResponseCompleteEvent may fire.
            request_->Fail(error_code, error_string);
        }
    }

private:
    mf::http::HttpConfig::ConstPointer http_config_;

    mf::http::HttpRequest::Pointer request_;

    ApiFunctor api_functor_;
    typename ApiFunctor::CallbackType cb_;

    IoService * callback_ios_;

    std::string content_;
    std::string hostname_;
    std::string url_;

    boost::optional<mf::http::Headers> headers_;
};


}  // namespace detail
}  // namespace api
}  // namespace mf
