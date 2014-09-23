/**
 * @file state_initialize.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "boost/asio/ssl.hpp"

#include "boost/msm/front/state_machine_def.hpp"

#include "mediafire_sdk/http/error.hpp"
#include "mediafire_sdk/http/url.hpp"
#include "mediafire_sdk/http/detail/http_request_events.hpp"

#include "mediafire_sdk/http/detail/socket_wrapper.hpp"

namespace mf {
namespace http {
namespace detail {

namespace msm = boost::msm;

struct Initializing : public msm::front::state<>
{
    template <typename FSM>
    void DoInit(
            FSM & fsm
        )
    {
        using mf::http::http_error;

        // We may be here due to new connection or due to a redirect, so
        // ensure we are initialized as if new.

        // We should not be connected at this point.
        fsm.Disconnect();

        // Parse URL if redirect didn't pass one in.
        try {
            if ( ! fsm.get_parsed_url() )
            {
                //fsm.parsed_url_.reset(new mf::http::Url(fsm.url_));
                fsm.set_parsed_url(
                    std::unique_ptr<mf::http::Url>(
                        new mf::http::Url(fsm.get_url())));
            }
        }
        catch(mf::http::InvalidUrl & err)
        {
            std::stringstream ss;
            ss << "Bad url(url:";
            ss << fsm.get_url();
            ss << " reason: " << err.what() << ")";

            fsm.ProcessEvent(ErrorEvent{
                    make_error_code( http_error::InvalidUrl ),
                    ss.str()
                });
            return;
        }

        if ( fsm.get_parsed_url()->scheme() == "http" )
        {
            fsm.set_is_ssl(false);

            // Create non-SSL socket and wrapper.
            fsm.set_socket_wrapper( std::make_shared<SocketWrapper>(
                new asio::ip::tcp::socket(*fsm.get_work_io_service()) ));

            fsm.ProcessEvent(InitializedEvent());
        }
        else if ( fsm.get_parsed_url()->scheme() == "https" )
        {
            fsm.set_is_ssl(true);

            // Create the SSL socket and wrapper
            fsm.set_socket_wrapper( std::make_shared<SocketWrapper>(
                new asio::ssl::stream<asio::ip::tcp::socket>(
                    *fsm.get_work_io_service(), *fsm.get_ssl_ctx() ) ));

            fsm.ProcessEvent(InitializedEvent());
        }
        else
        {
            std::stringstream ss;
            ss << "Unsupported scheme. Url: " << fsm.get_url();
            fsm.ProcessEvent(ErrorEvent{
                    make_error_code( http_error::InvalidUrl ),
                    ss.str()
                });
        }
    }

    template <typename Event, typename FSM>
    void on_entry(Event const&, FSM & fsm)
    {
        DoInit(fsm);
    }
};

}  // namespace detail
}  // namespace http
}  // namespace mf
