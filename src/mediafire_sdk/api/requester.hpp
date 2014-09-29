/**
 * @file requester.hpp
 * @author Herbert Jones
 * @brief http request wrapper for API calls.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <string>

#include <cstring>

#include "mediafire_sdk/api/detail/requester_impl.hpp"
#include "mediafire_sdk/api/error.hpp"
#include "mediafire_sdk/api/ptree_helpers.hpp"
#include "mediafire_sdk/api/types.hpp"
#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/http_config.hpp"

#include "boost/algorithm/string/predicate.hpp"  // iequals
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

namespace mf {
namespace api {

/**
 * @class Requester
 * @brief Encapsulates data required to initialize and make an API call.
 *
 * In order to easy use of API requests and not require that clients contain
 * boilerplate code, this class provides methods to start requests and pass the
 * response data to a callback.
 *
 * This class can be used instead of the SessionMaintainer if only making
 * API calls that do not require session tokens.
 */
class Requester
{
public:
    /**
     * @brief CTOR when only one thread is going to be used for API work and the
     * return callbacks.
     *
     * If callbacks should be done in the same thread as the one where API work
     * is done, then use this constructor. You can still override the default in
     * Call().
     *
     * @param[in] http_config HttpConfig object
     * @param[in] hostname The domain where all requests should go.
     */
    Requester(
            mf::http::HttpConfig::ConstPointer http_config,
            std::string hostname
            ) :
        http_config_(http_config),
        hostname_(hostname)
    {}

    virtual ~Requester() {}

    /**
     * @brief Request an API call to be performed.
     *
     * @param[in] af The API functor for the desired call.
     * @param[in] cb Callback for the API functor.
     * @param[in] callback_io_service The thread where callbacks should execute.
     * @param[in] start Start immediately or let caller start
     *
     * @return The HttpRequest object, which you can use to cancel the
     *         operation if needed.
     */
    template<typename ApiRequest, typename IoService>
    mf::http::HttpRequest::Pointer Call(
            ApiRequest af,
            typename ApiRequest::CallbackType cb,
            IoService * callback_io_service,
            RequestStarted start
        )
    {
        typedef detail::RequesterImpl<ApiRequest, IoService> Impl;

        assert(callback_io_service);

        std::shared_ptr<Impl> wrapper( Wrap( af, cb, callback_io_service ) );

        return wrapper->Init(start);
    }

    /**
     * @brief Request an API call to be performed.
     *
     * Callbacks will execute in the default handler thread.
     *
     * @param[in] af The API functor for the desired call.
     * @param[in] cb Callback for the API functor.
     * @param[in] start Allow caller to start request if additional
     * configuration to be done on HttpRequest before starting.
     *
     * @return The HttpRequest object, which you can use to cancel the
     *         operation if needed.
     */
    template<typename ApiRequest>
    mf::http::HttpRequest::Pointer Call(
            ApiRequest af,
            typename ApiRequest::CallbackType cb,
            RequestStarted start
        )
    {
        return Call(std::move(af), cb,
            http_config_->GetDefaultCallbackIoService(), start);
    }

    /**
     * @brief Request an API call to be performed.
     *
     * Blocks thread until work io_service handles request.  Due to this it must
     * not be called from that io_service thread or it will block forever.
     *
     * @param[in] af The API functor for the desired call.
     *
     * @return The response object of the ApiRequest request.
     *
     * @warning Do not call from work io_service thread.
     */
    template<typename ApiRequest>
    typename ApiRequest::ResponseType CallSynchronous(
            ApiRequest af
        )
    {
        typedef detail::RequesterImpl<ApiRequest, boost::asio::io_service>
            Impl;

        std::promise<typename ApiRequest::ResponseType> response_promise;
        auto future = response_promise.get_future();

        std::shared_ptr<Impl> wrapper(
            std::make_shared<Impl>(
                http_config_,
                af,
                [&response_promise](
                    const typename ApiRequest::ResponseType & response )
                {
                    response_promise.set_value(response);
                },
                http_config_->GetWorkIoService(),
                hostname_));

        wrapper->Init(RequestStarted::Yes);

        return future.get();
    }

    /**
     * @brief Request an API call but do not start it.
     *
     * @param[in] af The API functor for the desired call.
     * @param[in] cb Callback for the API functor.
     * @param[in] ios The thread where callbacks should execute.
     *
     * @return The HttpRequest object, which you can use to cancel the
     *         operation if needed.
     */
    template<typename ApiRequest, typename IoService>
    std::shared_ptr<detail::RequesterImpl<ApiRequest, IoService>> Wrap(
            ApiRequest af,
            typename ApiRequest::CallbackType cb,
            IoService * callback_ios
        )
    {
        typedef detail::RequesterImpl<ApiRequest, IoService> Impl;

        assert(callback_ios);

        std::shared_ptr<Impl> wrapper(
            std::make_shared<Impl>(
                http_config_,
                af,
                cb,
                callback_ios,
                hostname_));

        // Start the request.
        return wrapper;
    }

private:
    mf::http::HttpConfig::ConstPointer http_config_;

    std::string hostname_;
};

}  // namespace api
}  // namespace mf
