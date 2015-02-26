/**
 * @file api/unit_tests/ut_live_user.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "ut_live.hpp"

#ifndef OUTPUT_DEBUG
#  define OUTPUT_DEBUG
#endif

#include "mediafire_sdk/api/user/destroy_action_token.hpp"
#include "mediafire_sdk/api/user/fetch_tos.hpp"
#include "mediafire_sdk/api/user/get_action_token.hpp"
#include "mediafire_sdk/api/user/get_avatar.hpp"
#include "mediafire_sdk/api/user/get_info.hpp"
#include "mediafire_sdk/api/user/get_limits.hpp"
#include "mediafire_sdk/api/user/get_login_token.hpp"
#include "mediafire_sdk/api/user/get_session_token.hpp"
#include "mediafire_sdk/api/user/get_settings.hpp"
#include "mediafire_sdk/api/user/set_settings.hpp"
#include "mediafire_sdk/api/user/update.hpp"

#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include "boost/asio/impl/src.hpp"      // Define once in program
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#define BOOST_TEST_MODULE ApiLiveUser
#include "boost/test/unit_test.hpp"

namespace api = mf::api;

namespace globals {
using namespace ut::globals;
std::string action_token;
}  // namespace globals

BOOST_FIXTURE_TEST_SUITE( s, ut::Fixture )

BOOST_AUTO_TEST_CASE(SessionTokenOverSessionMaintainerLive)
{
    const api::credentials::Email & connection(globals::connection1);

    Call(
        api::user::get_session_token::Request(globals::connection1),
        [connection, this](const api::user::get_session_token::Response & response)
        {
            Stop();

            if ( response.error_code )
            {
                std::ostringstream ss;
                ss << "User: " << connection.email << std::endl;
                ss << "Password: " << connection.password << std::endl;
                ss << "Error: " << response.error_string << std::endl;
                BOOST_FAIL(ss.str());
            }
            else
            {
                BOOST_CHECK( ! response.session_token.empty() );
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(UserGetInfo)
{
    using api::user::get_info::Linked;

    Call(api::user::get_info::Request(),
         [&](const api::user::get_info::Response & response)
         {
             if (response.error_code)
             {
                 Fail(response);
             }
             else
             {
                 Log("Has facebook:",
                     response.facebook && response.facebook->linked == Linked::Yes);
                 Log("Has twitter:",
                     response.twitter && response.twitter->linked == Linked::Yes);
                 Log("Has gmail:",
                     response.gmail && response.gmail->linked == Linked::Yes);
                 Log("Email:", response.email);
                 Log("Display name:", response.display_name);
                 Log("Ekey:", response.ekey);

                 Success();
             }
         });
    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(GetSettings)
{
    Call(api::user::get_settings::Request(),
         [&](const api::user::get_settings::Response & response)
         {
             if (response.error_code)
             {
                 Fail(response);
             }
             else
             {
                 Success();
             }
         });
    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(SetSettings)
{
    api::user::set_settings::Request request;
    request.SetDefaultShareLinkStatus(
            api::user::set_settings::ShareLinkStatus::Enabled);

    Call(request, [&](const api::user::set_settings::Response & response)
         {
             if (response.error_code)
             {
                 Fail(response);
             }
             else
             {
                 Success();
             }
         });
    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(UpdateUser)
{
    api::user::update::Request request;
    request.SetGender(api::user::update::Gender::None);

    Call(request, [&](const api::user::update::Response & response)
         {
             if (response.error_code)
             {
                 Fail(response);
             }
             else
             {
                 Success();
             }
         });
    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(GetAvatar)
{
    Call(api::user::get_avatar::Request(),
         [&](const api::user::get_avatar::Response & response)
         {
             if (response.error_code)
             {
                 Fail(response);
             }
             else
             {
                 Log("Avatar URL:", response.avatar_url);

                 Success();
             }
         });
    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(SessionMaintainerRestartable)
{
    const api::credentials::Email & connection(globals::connection1);

    Call(
        api::user::get_session_token::Request(globals::connection1),
        [connection, this](const api::user::get_session_token::Response & response)
        {
            Stop();

            if ( response.error_code )
            {
                std::ostringstream ss;
                ss << "User: " << connection.email << std::endl;
                ss << "Password: " << connection.password << std::endl;
                ss << "Error: " << response.error_string << std::endl;
                BOOST_FAIL(ss.str());
            }
            else
            {
                BOOST_CHECK( ! response.session_token.empty() );
            }
        });

    StartWithDefaultTimeout();

    bool actuallyCalled = false;
    Call(
        api::user::get_session_token::Request(globals::connection1),
        [&actuallyCalled, connection, this](const api::user::get_session_token::Response & response)
        {
            actuallyCalled = true;
            Stop();

            if ( response.error_code )
            {
                std::ostringstream ss;
                ss << "User: " << connection.email << std::endl;
                ss << "Password: " << connection.password << std::endl;
                ss << "Error: " << response.error_string << std::endl;
                BOOST_FAIL(ss.str());
            }
            else
            {
                BOOST_CHECK( ! response.session_token.empty() );
            }
        });

    StartWithDefaultTimeout();

    BOOST_CHECK( actuallyCalled );
}

BOOST_AUTO_TEST_CASE(CredentialsChangeOverSessionMaintainerLive)
{
    const api::credentials::Email & connection1(globals::connection1);
    const api::credentials::Email & connection2(globals::connection2);

    // Start with default credentials
    Call(
        api::user::get_info::Request(),
        [&](const api::user::get_info::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                BOOST_CHECK_EQUAL( response.email, connection1.email );
                Stop();
            }
        });

    StartWithDefaultTimeout();

    // Call again with default credentials
    ChangeCredentials(connection1);
    Call(
        api::user::get_info::Request(),
        [&](const api::user::get_info::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                BOOST_CHECK_EQUAL( response.email, connection1.email );
                Stop();
            }
        });

    StartWithDefaultTimeout();

    // Call with new credentials
    ChangeCredentials(connection2);
    Call(
        api::user::get_info::Request(),
        [&](const api::user::get_info::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                BOOST_CHECK_EQUAL( response.email, connection2.email );
                Stop();
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(UserGetLimits)
{
    Call(
        api::user::get_limits::Request(),
        [&](const api::user::get_limits::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(GetActionToken)
{
    api::user::get_action_token::Request request(
        api::user::get_action_token::Type::Image);
    Call(
        request,
        [&](const api::user::get_action_token::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                globals::action_token = response.action_token;
                Success();
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(DestroyActionToken)
{
    Call(
        api::user::destroy_action_token::Request(globals::action_token),
        [&](const api::user::destroy_action_token::Response & response)
        {
            if ( response.error_code )
                Fail(response);
            else
                Success();
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(GetLoginToken)
{
    api::credentials::Email email_creds = {ut::user1::username, ut::user1::password};
    Call(
        api::user::get_login_token::Request(email_creds),
        [&](const api::user::get_login_token::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();
                std::cout << "Got login token: "
                          << "https://" << ut::constants::host
                          << "/api/user/login_with_token.php?login_token="
                          << response.login_token << std::endl;
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FetchTOS)
{
    namespace tos = api::user::fetch_tos;
    Call(tos::Request(), [&](const tos::Response & response)
         {
             if (response.error_code)
             {
                 Fail(response);
             }
             else
             {
                 Log("User accepted terms:",
                     response.user_accepted_terms == tos::TOSAccepted::Yes);
                 Log("Acceptance token:", response.acceptance_token);
                 Success();
             }
         });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(EkeyTest)
{
    api::credentials::Email email_creds = {ut::user1::username, ut::user1::password};
    boost::optional<api::credentials::Ekey> ekey_creds;
    Call(
        api::user::get_session_token::Request(email_creds),
        [&](const api::user::get_session_token::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                std::cout << "Got ekey: " << response.ekey << std::endl;
                ekey_creds = api::credentials::Ekey{response.ekey, ut::user1::password};
                Success();
            }
        });

    StartWithDefaultTimeout();

    // Now use the ekey
    BOOST_REQUIRE( ekey_creds );

    Call(
        api::user::get_session_token::Request(*ekey_creds),
        [&](const api::user::get_session_token::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                std::cout << "Logged in with ekey!" << std::endl;
                Success();
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_SUITE_END()
