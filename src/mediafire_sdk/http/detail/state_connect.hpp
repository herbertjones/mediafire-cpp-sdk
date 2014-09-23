/**
 * @file state_connect.hpp
 * @author Herbert Jones
 * @brief Config state machine transitions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include "boost/bind.hpp"
#include "boost/asio.hpp"
#include "boost/msm/front/state_machine_def.hpp"

#include "mediafire_sdk/http/detail/http_request_events.hpp"
#include "mediafire_sdk/http/detail/race_preventer.hpp"
#include "mediafire_sdk/http/detail/timeouts.hpp"
#include "mediafire_sdk/http/error.hpp"

namespace mf {
namespace http {
namespace detail {

class ConnectData
{
public:
    ConnectData() :
        cancelled(false)
    {}

    bool cancelled;
};

using ConnectDataPointer = std::shared_ptr<ConnectData>;

template <typename FSM>
void HandleConnect(
        FSM & fsm,
        ConnectDataPointer state_data,
        RacePreventer race_preventer,
        const boost::system::error_code& err
    )
{
    // Stop processing if actions cancelled.
    if (state_data->cancelled == true)
        return;

    using mf::http::http_error;

    // Skip if cancelled due to timeout.
    if ( ! race_preventer.IsFirst() ) return;

    fsm.ClearAsyncTimeout();  // Must stop timeout timer.

    if (!err)
    {
        fsm.ProcessEvent(ConnectedEvent{});
    }
    else
    {
        std::stringstream ss;
        ss << "Failure while connecting(" << fsm.get_url() << ").";
        ss << " Error: " << err.message();
        fsm.ProcessEvent(
            ErrorEvent{
            make_error_code(
                http_error::UnableToConnect ),
            ss.str()
            });
    }
}

class Connect : public boost::msm::front::state<>
{
public:
    template <typename FSM>
    void on_entry(ResolvedEvent const & evt, FSM & fsm)
    {
        auto state_data = std::make_shared<ConnectData>();
        state_data_ = state_data;

        // Must prime timeout for async actions.
        auto race_preventer = fsm.SetAsyncTimeout("connect", kConnectTimeout);

        auto socket_wrapper = fsm.get_socket_wrapper();
        auto fsmp = fsm.AsFrontShared();

        if ( fsm.get_is_ssl() )
        {
            assert(socket_wrapper->SslSocket());

            // Attempt a connection to each endpoint in the list until we
            // successfully establish a connection.
            boost::asio::async_connect(
                socket_wrapper->SslSocket()->lowest_layer(),
                evt.endpoint_iterator,
                [fsmp, state_data, race_preventer](
                        const boost::system::error_code& ec,
                        boost::asio::ip::tcp::resolver::iterator iterator
                    )
                {
                    HandleConnect(*fsmp, state_data, race_preventer, ec);
                });
        }
        else
        {
            assert(socket_wrapper->Socket());

            // Attempt a connection to each endpoint in the list until we
            // successfully establish a connection.
            boost::asio::async_connect(
                *socket_wrapper->Socket(),
                evt.endpoint_iterator,
                [fsmp, state_data, race_preventer](
                        const boost::system::error_code& ec,
                        boost::asio::ip::tcp::resolver::iterator iterator
                    )
                {
                    HandleConnect(*fsmp, state_data, race_preventer, ec);
                });
        }
    }

    template <typename Event, typename FSM>
    void on_exit(Event const&, FSM &)
    {
        state_data_->cancelled = true;
        state_data_.reset();
    }

private:
    ConnectDataPointer state_data_;
};

}  // namespace detail
}  // namespace http
}  // namespace mf
