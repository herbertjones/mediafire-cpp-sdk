/**
 * @file state_ssl_handshake.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <sstream>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/msm/front/state_machine_def.hpp"

#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"
#include "mediafire_sdk/http/detail/timeouts.hpp"
#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/url.hpp"

namespace mf {
namespace http {
namespace detail {

namespace {

#define CASE(X) case X: error_msg=#X; break;
void fprint_openssl_error(FILE* file, int error)
{
    fprintf(file, "Error: ");
    std::string error_msg;
    switch (error)
    {
        CASE(X509_V_OK)
        CASE(X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT)
        CASE(X509_V_ERR_UNABLE_TO_GET_CRL)
        CASE(X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE)
        CASE(X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE)
        CASE(X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY)
        CASE(X509_V_ERR_CERT_SIGNATURE_FAILURE)
        CASE(X509_V_ERR_CRL_SIGNATURE_FAILURE)
        CASE(X509_V_ERR_CERT_NOT_YET_VALID)
        CASE(X509_V_ERR_CERT_HAS_EXPIRED)
        CASE(X509_V_ERR_CRL_NOT_YET_VALID)
        CASE(X509_V_ERR_CRL_HAS_EXPIRED)
        CASE(X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD)
        CASE(X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD)
        CASE(X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD)
        CASE(X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD)
        CASE(X509_V_ERR_OUT_OF_MEM)
        CASE(X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT)
        CASE(X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN)
        CASE(X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY)
        CASE(X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE)
        CASE(X509_V_ERR_CERT_CHAIN_TOO_LONG)
        CASE(X509_V_ERR_CERT_REVOKED)
        CASE(X509_V_ERR_APPLICATION_VERIFICATION)
        default:
            error_msg = "Unknown error: " + mf::utils::to_string(error);
    }
    fprintf(file, "%s\n", error_msg.c_str());
}
#undef CASE

class DebugCertificateVerifier
{
public:
    DebugCertificateVerifier(std::string hostname) :
        hostname_(std::move(hostname))
    {
        // Empty
    }

    bool operator()(bool preVerified, boost::asio::ssl::verify_context& ctx)
    {
        X509_STORE_CTX* x509_ctx = ctx.native_handle();

#ifdef OUTPUT_DEBUG
        X509* cert = x509_ctx->cert;
        fprintf(stdout, "Validating Certificate issued by:\n");
        X509_NAME_print_ex_fp(
                stdout,
                cert->cert_info->issuer,
                2,
                XN_FLAG_RFC2253
            );
        fprintf(stdout, "\n");
        fprintf(stdout, "Issued to:\n");
        X509_NAME_print_ex_fp(
                stdout,
                cert->cert_info->subject,
                2,
                XN_FLAG_RFC2253
            );
        fprintf(stdout, "\n");
#endif

        bool returnValue =
            boost::asio::ssl::rfc2818_verification(hostname_)(preVerified, ctx);

        if ( X509_STORE_CTX_get_error(x509_ctx) != 0 )
        {
            fprintf(stdout, "Verification failed!\n");
            X509* failed_cert = X509_STORE_CTX_get_current_cert(x509_ctx);
            if ( failed_cert != nullptr )
            {
                fprintf(stdout, "At certificate:\n");
                X509_NAME_print_ex_fp(
                        stdout,
                        failed_cert->cert_info->subject,
                        2,
                        XN_FLAG_RFC2253
                    );
                fprintf(stdout, "\n");
            }
            fprint_openssl_error(stdout, X509_STORE_CTX_get_error(x509_ctx));
        }

        return returnValue;
    }

private:
    std::string hostname_;
};

}  // anonymous namespace

class SslConnectData
{
public:
    SslConnectData() :
        cancelled(false)
    {}

    bool cancelled;
};
using SslConnectDataPointer = std::shared_ptr<SslConnectData>;

template <typename FSM>
void HandleHandshake(
        FSM  & fsm,
        SslConnectDataPointer state_data,
        RacePreventer race_preventer,
        const boost::system::error_code& err
    )
{
    using mf::http::http_error;

    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

    // Skip if cancelled due to timeout.
    if ( ! race_preventer.IsFirst() ) return;

    fsm.ClearAsyncTimeout();  // Must stop timeout timer.

    if (!err)
    {
        fsm.ProcessEvent(HandshakeEvent{});
    }
    else
    {
        std::stringstream ss;
        ss << "Failure in SSL handshake.";
        ss << " Error: " << err.message();
        fsm.ProcessEvent(
            ErrorEvent{
                make_error_code(
                    http_error::SslHandshakeFailure ),
                ss.str()
            });
    }
}

class SSLHandshake : public boost::msm::front::state<>
{
public:
    template <typename Event, typename FSM>
    void on_entry(Event const &, FSM & fsm)
    {
        auto state_data = std::make_shared<SslConnectData>();
        state_data_ = state_data;

        auto ssl_socket = fsm.get_socket_wrapper()->SslSocket();
        const Url * url = fsm.get_parsed_url();

        // This disables the Nagle algorithm, as supposedly SSL already
        // does something similar and Nagle hurts SSL performance by a lot,
        // supposedly.
        // http://www.extrahop.com/post/blog/performance-metrics/to-nagle-or-not-to-nagle-that-is-the-question/
        ssl_socket->lowest_layer().set_option(asio::ip::tcp::no_delay(true));

        // Don't allow self signed certs.
        ssl_socket->set_verify_mode(fsm.get_ssl_verify_mode());

        // Properly walk the certificate chain.
        ssl_socket->set_verify_callback(
                DebugCertificateVerifier(url->host())
            );

        // Must prime timeout for async actions.
        auto race_preventer = fsm.SetAsyncTimeout("ssl handshake",
            kSslHandshakeTimeout);
        auto fsmp = fsm.AsFrontShared();

        ssl_socket->async_handshake(
            boost::asio::ssl::stream_base::client,
            [fsmp, state_data, race_preventer](
                    const boost::system::error_code& ec
                )
            {
                HandleHandshake(*fsmp, state_data, race_preventer, ec);
            });

    }

    template <typename Event, typename FSM>
    void on_exit(Event const&, FSM &)
    {
        state_data_->cancelled = true;
        state_data_.reset();
    }

private:
    SslConnectDataPointer state_data_;
};

}  // namespace detail
}  // namespace http
}  // namespace mf
