/**
 * @file http_config.hpp
 * @author Herbert Jones
 * @brief Configuration for HttpRequest objects
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <iostream>
#include <memory>

#include "boost/optional.hpp"

#include "mediafire_sdk/http/bandwidth_analyser_interface.hpp"
#include "mediafire_sdk/utils/forward_declarations/asio.hpp"

namespace mf {
namespace http {

using HeaderContainer = std::vector< std::pair<std::string, std::string> >;

enum class RedirectPolicy
{
    Deny,
    DenyDowngrade,
    Allow
};
enum class SelfSigned
{
    Denied,
    Permitted
};

/**
 * @struct Proxy
 * @brief Proxy data
 */
struct Proxy
{
    std::string host;
    uint16_t port;
    std::string username;
    std::string password;
};

/**
 * @class HttpConfig
 * @brief Configuration for HttpRequest objects
 *
 * Will create a default io_service if none provided which must be run in order
 * to work.
 *
 * @warning If placing long running tasks in callbacks or on the work
 * io_service, http requests can time out, as they won't be handled before the
 * connection times out.  If you believe this may happen due to any long running
 * processes, then use a separate thread to run the io_service.
 *
 * Creates a default BandwidthAnalyser if none set.
 */
class HttpConfig : public std::enable_shared_from_this<HttpConfig>
{
public:
    using Pointer = std::shared_ptr<HttpConfig>;
    using ConstPointer = std::shared_ptr<const HttpConfig>;

    /**
     * @brief Create an HttpRequest with default parameters.
     */
    static Pointer Create();

    /**
     * @brief Create an HttpRequest.
     *
     * @param[in] io_service The ASIO io_service to use to perform all actions
     * and callbacks.
     */
    static Pointer Create(boost::asio::io_service * io_service);

    /**
     * @brief Create an HttpRequest.
     *
     * @param[in] io_service The ASIO io_service to use to perform actions.
     * @param[in] io_service The ASIO io_service to use to perform callbacks.
     */
    static Pointer Create(
            boost::asio::io_service * io_service,
            boost::asio::io_service * callback_io_service
        );
    ~HttpConfig();

    /**
     * @brief Create a copy of the HttpConfig.
     *
     * This creates a copy of the current HttpConfig object.  Avoid using the
     * default io_services in that case, though it is not impossible to do so.
     * Using a clone is useful for making small changes and testing.
     *
     * @return Copy of this HttpConfig.
     */
    Pointer Clone() const;

    /**
     * @brief Set the io_service for work operations.
     *
     * @warning It is not safe to change this after operations have started.
     *
     * @param[in] io_service The Asio io_service object pointer to use.
     */
    void SetWorkIoService(boost::asio::io_service * io_service);

    /**
     * @brief Get a pointer to the io_service object to use.
     *
     * If no io_service object has been passed in via SetWorkIoService, a
     * default will be created.  RunService must be called to run the
     * io_service in that case.
     *
     * @return io_service where http requests will be made.
     */
    boost::asio::io_service * GetWorkIoService() const;

    /**
     * @brief Set the default io_service for callbacks.  Normally the callback
     * io_service can be overridden when passing in a callback.  If none is
     * provided at that point, this will be used.
     *
     * @warning It is not safe to change this after operations have started.
     *
     * @param[in] io_service The Asio io_service object pointer to use.
     */
    void SetDefaultCallbackIoService(boost::asio::io_service *);

    /**
     * @brief Get a pointer to the io_service object to use for callbacks.
     *
     * If no io_service object has been passed in via
     * SetDefaultCallbackIoService, the work io_service will be used.
     *
     * @return io_service where http requests will be made.
     */
    boost::asio::io_service * GetDefaultCallbackIoService() const;

    /**
     * @brief Set the BandwidthAnalyserInterface for http operations.
     *
     * @warning It is not safe to change this after operations have started.
     *
     * @param[in] bandwidth_analyser The BandwidthAnalyserInterface object
     * pointer to use.
     */
    void SetBandwidthAnalyser(BandwidthAnalyserInterface::Pointer bandwidth_analyser);

    /**
     * @brief Get a pointer to the BandwidthAnalyserInterface object used to
     * track bandwidth.
     *
     * If no BandwidthAnalyserInterface object has been passed in via
     * SetBandwidthAnalyser, a default will be created.
     *
     * @return io_service where http requests will be made.
     */
    BandwidthAnalyserInterface::Pointer GetBandwidthAnalyser() const;

    /**
     * @brief Get a pointer to the SSL context.
     *
     * If no context has been passed in via SetSslContext, a default will be
     * created and used.
     *
     * @return Asio SSL context to use for HTTP requests.
     */
    boost::asio::ssl::context * GetSslContext() const;

    /**
     * @brief Set the SSL context.
     *
     * If the defaults need to be overridden, the SSL context can be set here.
     *
     * @param[in] ctx Shared pointer to SSL context.
     */
    void SetSslContext(std::shared_ptr<boost::asio::ssl::context> ctx);

    /**
     * @brief Convenience function to run the io_service.
     */
    void RunService();

    /**
     * @brief Convenience function to stop the io_service.
     */
    void StopService();

    /**
     * @brief The HTTP proxy if one is set.
     *
     * @return Possible proxy if set.
     */
    boost::optional<Proxy> GetHttpProxy() const {return http_proxy_;}

    /**
     * @brief Set the HTTP proxy.
     *
     * @param[in] New proxy to use.
     */
    void SetHttpProxy(const Proxy & proxy) {http_proxy_ = proxy;}

    /**
     * @brief Remove the HTTP proxy.
     */
    void RemoveHttpProxy() {http_proxy_ = boost::none;}

    /**
     * @brief The HTTPS proxy if one is set.
     *
     * @return Possible proxy if set.
     */
    boost::optional<Proxy> GetHttpsProxy() const {return https_proxy_;}

    /**
     * @brief Set the HTTPS proxy.
     *
     * @param[in] New proxy to use.
     */
    void SetHttpsProxy(const Proxy & proxy) {https_proxy_ = proxy;}

    /**
     * @brief Remove the HTTPS proxy.
     */
    void RemoveHttpsProxy() {https_proxy_ = boost::none;}

    /**
     * @brief Get the current setting for self signed SSL certificates.
     *
     * The default is to deny self signed certificates.
     *
     * @return Permitted if self signed certificates are allowed.
     */
    SelfSigned SelfSignedCertificatesAllowed() const
    {return self_signed_certs_allowed_;}

    /**
     * @brief Allow or deny self signed certificates.
     *
     * The default is to deny self signed certificates.
     *
     * @param[in] self_signed New setting for self signed certificates.
     */
    void AllowSelfSignedCertificate(
            SelfSigned self_signed = SelfSigned::Permitted
        )
    {
        self_signed_certs_allowed_=self_signed;
    }

    /**
     * @brief Get the current redirect policy.
     *
     * The default is to allow redirects.
     *
     * @return Current redirect policy.
     */
    RedirectPolicy GetRedirectPolicy() const {return redirect_policy_;}

    /**
     * @brief Set the redirect policy.
     *
     * @param[in] policy The new redirect policy.
     */
    void SetRedirectPolicy(RedirectPolicy policy) {redirect_policy_=policy;}

    /**
     * @brief Get the default headers.
     *
     * @return The headers that will be passed with all HTTP(S) requests.
     */
    HeaderContainer GetDefaultHeaders() const {return default_headers_;}

    /**
     * @brief Change the headers that are used in all requests.
     *
     * @param[in] headers New default headers.
     */
    void SetDefaultHeaders(HeaderContainer headers) {default_headers_=headers;}

    /**
     * @brief Set the percentage of the bandwidth we should consume at most.
     *
     * This is a simple QOS setting, which only affects POST uploads and
     * content downloads. The default percent is 100, which means to perform
     * no delays in reading and writing to the socket.
     *
     * @param[in] percentage The percentage of bandwidth to try to limit
     *                       ourselves to. Valid range: 1-100.
     */
    void SetBandwidthUsagePercent(uint32_t v) {bandwidth_usage_percent_=v;}

    /**
     * @brief Get the current percentage of bandwidth to be consumed.
     *
     * @return The percentage of bandwidth to use as an integer.
     */
    uint32_t GetBandwidthUsagePercent() const {return bandwidth_usage_percent_;}

private:
    HttpConfig();

    mutable boost::asio::io_service * io_service_;
    mutable std::shared_ptr<boost::asio::io_service> owned_io_service_;

    boost::asio::io_service * default_callback_io_service_;

    BandwidthAnalyserInterface::Pointer bandwidth_analyser_;

    mutable std::shared_ptr<boost::asio::ssl::context> ssl_ctx_;

    boost::optional<Proxy> http_proxy_;
    boost::optional<Proxy> https_proxy_;

    SelfSigned self_signed_certs_allowed_;

    RedirectPolicy redirect_policy_;

    HeaderContainer default_headers_;

    uint32_t bandwidth_usage_percent_;
};

}  // namespace http
}  // namespace mf
