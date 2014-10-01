/**
 * @file up_multithread.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include <cstdlib>
#include <iostream>
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <memory>

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#  include "boost/asio/impl/src.hpp"  // Define once in program
#  include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif
#include "boost/thread/thread.hpp"
#include "boost/variant/get.hpp"

#include "mediafire_sdk/api/session_maintainer.hpp"
#include "mediafire_sdk/api/user/get_info.hpp"

namespace constants {
const std::string host = "www.mediafire.com";
const int threads_to_run = 4;
const int calls_to_make = 100;
}  // namespace constants
namespace user1 {
const std::string username = TEST_USER_1_USERNAME;
const std::string password = TEST_USER_1_PASSWORD;
}  // namespace user1
namespace user2 {
const std::string username = TEST_USER_2_USERNAME;
const std::string password = TEST_USER_2_PASSWORD;
}  // namespace user2

namespace asio = boost::asio;

int main(int argc, char *argv[])
{
    int return_code = 1;

    asio::io_service io_service;
    boost::thread_group thread_pool;
    asio::io_service::work work(io_service);

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);

    mf::api::SessionMaintainer stm(http_config);

    // Handle session token failures.
    stm.SetSessionStateChangeCallback(
        [&io_service](mf::api::SessionState state)
        {
            if (boost::get<mf::api::session_state::CredentialsFailure>(
                    &state))
        {
            std::cout << "Username or password incorrect."
                << std::endl;
            io_service.stop();
        }
        });

    stm.SetLoginCredentials( mf::api::credentials::Email{ user1::username,
        user1::password } );

    // Make several threads
    for (int i = 0; i < constants::threads_to_run; ++i)
    {
        thread_pool.create_thread(boost::bind(&asio::io_service::run,
                &io_service));
    }

    // Make several calls
    boost::mutex mutex;
    int calls_made = 0;
    auto callback = [&io_service, &calls_made, &mutex, &return_code](
            const mf::api::user::get_info::Response & response
        )
    {
            boost::lock_guard<boost::mutex> lock(mutex);
            ++calls_made;
            if (calls_made == constants::calls_to_make)
            {
                io_service.stop();
                return_code = 0;
            }

            std::cout << "Got response: " << response.debug << std::endl;
    };

    for (int i = 0; i < constants::calls_to_make; ++i)
    {
        stm.Call(
            mf::api::user::get_info::Request(),
            callback);
    }

    thread_pool.join_all();

    return return_code;
}

