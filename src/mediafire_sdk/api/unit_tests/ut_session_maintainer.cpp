/**
 * @file ut_session_maintainer.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include <limits>
#include <map>
#include <string>
#include <vector>

#include "mediafire_sdk/api/billing/get_products.hpp"

#include "mediafire_sdk/api/user/get_session_token.hpp"
#include "mediafire_sdk/api/user/get_action_token.hpp"

#include "mediafire_sdk/api/folder/get_content.hpp"

#include "mediafire_sdk/utils/md5_hasher.hpp"
#include "mediafire_sdk/utils/string.hpp"

#include "boost/algorithm/string/find.hpp"

#include "boost/asio.hpp"
#include "boost/asio/impl/src.hpp"  // Define once in program
#include "boost/asio/ssl.hpp"
#include "boost/asio/ssl/impl/src.hpp"  // Define once in program

#include "boost/date_time/posix_time/ptime.hpp"
#include "boost/format.hpp"

#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "mediafire_sdk/api/unit_tests/session_token_test_server.hpp"
#include "mediafire_sdk/api/unit_tests/api_ut_helpers.hpp"

#include "mediafire_sdk/api/session_maintainer.hpp"
#define BOOST_TEST_MODULE SessionMaintainer
#include "boost/test/unit_test.hpp"

namespace api = mf::api;

namespace {

const std::string address = "127.0.0.1";
const std::string port = "60259";
const std::string host = address + ":" + port;

#if ! defined(TEST_USER_1_USERNAME) || ! defined(TEST_USER_1_PASSWORD)
# error "TEST_USER defines not set."
#endif

const std::string username = TEST_USER_1_USERNAME;
const std::string password = TEST_USER_1_PASSWORD;

enum Regenerate
{
    R_No,
    R_Yes
};

std::string TestName()
{
    return boost::unit_test::framework::current_test_case().p_name;
}

boost::filesystem::path TemplateDir()
{
    boost::filesystem::path path(__FILE__);
    return path.parent_path();
}

std::string GetTemplate(std::string template_path)
{
    using std::ios;

    boost::filesystem::path path(TemplateDir()/template_path);
    boost::filesystem::ifstream ifs;

    ifs.open(path, ios::binary | ios::in );
    if ( ifs.is_open() && ifs.good() )
    {
        std::string contents(
            (std::istreambuf_iterator<char>(ifs)),
            (std::istreambuf_iterator<char>()   ));

        if ( ifs.eof() || ifs.good() )
            return contents;
    }

    std::ostringstream ss;
    ss << "Missing template: " << path.string();
    throw std::runtime_error(ss.str());
}

std::string InvalidSignatureResponse(
        std::string action
    )
{
    boost::property_tree::wptree pt = api::ut::MakeErrorApiContent(
            action,
            "The signature you specified is invalid",
            127 );
    return api::ut::ToString(pt);
}

}  // namespace

class FailTimer
{
public:
    FailTimer(
            boost::asio::io_service * ios,
            int timeout_seconds
        ) :
        io_service_(ios),
        timer_(*ios),
        timeout_seconds_(timeout_seconds)
    {
        io_service_->post(
            std::bind( &FailTimer::Run, this ) );
    }

    void Stop()
    {
        timer_.cancel();
    }

private:
    boost::asio::io_service * io_service_;
    boost::asio::deadline_timer timer_;

    int timeout_seconds_;

    void Run()
    {
        timer_.expires_from_now(
            boost::posix_time::seconds( timeout_seconds_ ) );
        timer_.async_wait(
            boost::bind(
                &FailTimer::HandleTimeout,
                this,
                boost::asio::placeholders::error
            )
        );
    }

    void HandleTimeout(const boost::system::error_code & err)
    {
        if (!err)
        {
            std::ostringstream ss;
            ss << "Timeout failure.";
            BOOST_FAIL(ss.str());
        }
    }
};



template<typename T>
class PHBinder
{
public:
    explicit PHBinder(T * t) :
        class_(t)
    {
    }

    void operator()(api::ut::RequestPointer rp)
    {
        class_->AcceptRequest( std::move(rp) );
    }

private:
    T * class_;
};

template<typename T>
api::ut::PathHandler PHBind(T * t)
{
    PHBinder<T> bound(t);
    return bound;
}

class SessionTokenWallet
{
public:
    struct SessionToken
    {
        std::string token;
        std::string pkey;
        std::string time;
        int secret_key;
    };

    bool ValidateSignature(
            api::ut::Request * request,
            Regenerate regenerate_on_success )
    {
        if ( DoValidateSignature( request, regenerate_on_success ) )
            return true;

        std::cout << "Request INVALID: " << request->http_request_path
            << std::endl;

        return false;
    }

    bool GetValidates(
            api::ut::Request * request,
            Regenerate regenerate_on_success
        )
    {
        std::string token, signature;
        {
            auto it = request->query_parts.find("session_token");
            if ( it == request->query_parts.end() )
                return false;
            token = it->second;

            it = request->query_parts.find("signature");
            if ( it == request->query_parts.end() )
                return false;
            signature = it->second;
        }

        // Can't be good if we can't find the token.
        auto token_it = tokens_.find(token);
        if ( token_it == tokens_.end() )
            return false;

        std::string::size_type sigpos = request->http_request_path.find(
            "signature=");
        if ( sigpos == std::string::npos )
            return false;
        if (sigpos > 0)
            --sigpos;

        std::ostringstream ss;
        ss << ( token_it->second.secret_key % 256 );
        ss << token_it->second.time;
        ss << request->http_request_path.substr(0, sigpos);

        if ( signature != mf::utils::HashMd5(ss.str()) )
            return false;

        if ( regenerate_on_success == R_Yes )
            GenerateNextSecretKey(token);

        return true;
    }

    bool PostValidates(
            api::ut::Request * request,
            Regenerate regenerate_on_success
        )
    {
        std::string token, signature;
        {
            auto it = request->post_parts.find("session_token");
            if ( it == request->post_parts.end() )
            {
                return false;
            }
            token = it->second;

            it = request->post_parts.find("signature");
            if ( it == request->post_parts.end() )
            {
                return false;
            }
            signature = it->second;
        }

        // Can't be good if we can't find the token.
        auto token_it = tokens_.find(token);
        if ( token_it == tokens_.end() )
        {
            return false;
        }

        std::ostringstream ss;

        // Signature is created using secret key + time + path + post
        ss << ( token_it->second.secret_key % 256 );
        ss << token_it->second.time;

        {
            std::string::size_type sigpos = request->http_request_path.find(
                "?");
            if ( sigpos == std::string::npos )
            {
                ss << request->http_request_path;
            }
            else
            {
                ss << request->http_request_path.substr(0, sigpos);
            }
            ss << "?";
        }

        {
            std::string::size_type sigpos = request->post.find(
                "signature=");
            if ( sigpos == std::string::npos )
            {
                return false;
            }
            if (sigpos > 0)
                --sigpos;
            ss << request->post.substr(0, sigpos);
        }

        if ( signature != mf::utils::HashMd5(ss.str()) )
        {
            return false;
        }

        if ( regenerate_on_success == R_Yes )
            GenerateNextSecretKey(token);

        return true;
    }

    bool DoValidateSignature(
            api::ut::Request * request,
            Regenerate regenerate_on_success )
    {
        return ( GetValidates(request, regenerate_on_success)
            || PostValidates(request, regenerate_on_success) );
    }

    void GenerateNextSecretKey(std::string token)
    {
        auto it = tokens_.find(token);
        if ( it != tokens_.end() )
        {
            uint64_t current_key = it->second.secret_key;
            uint64_t next_key = (current_key * 16807) % 2147483647;
            it->second.secret_key = static_cast<int>(next_key);
        }
    }

    SessionToken CreateValidToken()
    {
        static boost::random::random_device rng;
        SessionToken st;

        static const std::string alphanum_chars(
                "abcdefghijklmnopqrstuvwxyz"
                "0123456789"
                );
        static boost::random::uniform_int_distribution<> alphanum_dist(
            0, alphanum_chars.size() - 1);

        // token
        const uint32_t session_token_length = 144;
        st.token.reserve(session_token_length);
        for (uint32_t i = 0; i < session_token_length; ++i)
            st.token.push_back( alphanum_chars[alphanum_dist(rng)] );

        // pkey
        // This should stay the same unless the user changes the password.
        static bool pkey_generated = false;
        static std::string pkey;
        if ( ! pkey_generated )
        {
            pkey_generated = true;
            const uint32_t pkey_length = 10;
            pkey.reserve(pkey_length);
            for (uint32_t i = 0; i < pkey_length; ++i)
                pkey.push_back( alphanum_chars[alphanum_dist(rng)] );
        }
        st.pkey = pkey;

        // secret key
        static boost::random::uniform_int_distribution<> secret_key_dist(
            0, std::numeric_limits<int32_t>::max() );
        st.secret_key = secret_key_dist(rng);

        // time
        {
            boost::posix_time::ptime now(
                boost::posix_time::microsec_clock::universal_time());
            boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
            boost::posix_time::time_duration since_epoch = now - epoch;
            boost::posix_time::time_duration::sec_type total_seconds =
                since_epoch.total_seconds();
            boost::posix_time::time_duration::sec_type milliseconds =
                (since_epoch - boost::posix_time::seconds(total_seconds)
                    ).total_milliseconds();
            std::ostringstream ss;
            ss << total_seconds << "."
                << std::setw(4) << std::setfill('0') << milliseconds;
            st.time = ss.str();
        }

        tokens_[st.token] = st;

        return st;
    }

    void AddValidToken(
            std::string token,
            std::string pkey,
            std::string time,
            int secret_key
        )
    {
        SessionToken td;
        td.token      = token;
        td.pkey       = pkey;
        td.time       = time;
        td.secret_key = secret_key;

        tokens_[token] = std::move(td);
    }

    void InvalidateToken(std::string token)
    {
        std::cout << "Token INVALIDATED: " << token << std::endl;
        tokens_.erase(token);
    }

private:
    std::map<std::string, SessionToken> tokens_;
};
SessionTokenWallet session_token_wallet;

class SessionTokenServer
{
public:
    SessionTokenServer() :
        tokens_served_(0)
    {
    }

    void AcceptRequest(api::ut::RequestPointer request)
    {
        std::cout << "Serving token!" << std::endl;

        SessionTokenWallet::SessionToken st =
            session_token_wallet.CreateValidToken();

        ++tokens_served_;

        boost::property_tree::wptree pt =
            api::ut::MakeBaseApiContent("user/get_session_token");
        pt.put(L"response.session_token", mf::utils::bytes_to_wide(st.token) );
        pt.put(L"response.secret_key", mf::utils::bytes_to_wide(mf::utils::to_string(st.secret_key)) );
        pt.put(L"response.time", mf::utils::bytes_to_wide(st.time) );
        pt.put(L"response.pkey", mf::utils::bytes_to_wide(st.pkey) );

        request->Respond(api::ut::ok, api::ut::ToString(pt));
    }

    int tokens_served() const {return tokens_served_;}

private:
    int tokens_served_;
};

class GetActionTokenServer
{
public:
    GetActionTokenServer()
    {
        at_response_ = GetTemplate(
            "templates/user/get_action_token_image.txt");
    }

    void AcceptRequest(api::ut::RequestPointer request)
    {
        if ( ! session_token_wallet.ValidateSignature( request.get(), R_Yes ) )
        {
            request->Respond(api::ut::forbidden,
                InvalidSignatureResponse("folder/get_content"));
            return;
        }

        std::string default_str;
        std::string * to_send = &default_str;

        to_send = &at_response_;

        request->Respond(api::ut::ok, *to_send);
    }

private:
    std::string at_response_;
};

class FolderGetContentServer
{
public:
    FolderGetContentServer()
    {
        files_response_ = GetTemplate(
            "templates/folder/get_content/file_1.txt");

        folders_response_ = GetTemplate(
            "templates/folder/get_content/folder_1.txt");
    }

    void AcceptRequest(api::ut::RequestPointer request)
    {
        if ( ! session_token_wallet.ValidateSignature( request.get(), R_Yes ) )
        {
            request->Respond(api::ut::forbidden,
                InvalidSignatureResponse("folder/get_content"));
            return;
        }

        std::string default_str;
        std::string * to_send = &default_str;

        auto it = request->query_parts.find("content_type");
        if ( it != request->query_parts.end()
            && it->second == "files" )
        {
            to_send = &files_response_;
        }
        else
        {
            to_send = &folders_response_;
        }

        request->Respond(api::ut::ok, *to_send);
    }

private:
    std::string files_response_;
    std::string folders_response_;
};

class SessionTokenSimpleResponder
{
public:
    SessionTokenSimpleResponder(
            api::ut::HttpStatusCode status_code,
            std::string response
        ) :
        status_code_(status_code),
        response_(std::move(response))
    {
    }

    void AcceptRequest(api::ut::RequestPointer request)
    {
        request->Respond(status_code_, response_);
    }

private:
    api::ut::HttpStatusCode status_code_;
    std::string response_;
};

BOOST_AUTO_TEST_CASE(FolderGetContentFolders)
{
    boost::asio::io_service io_service;
    FailTimer ft(&io_service, 2);

    api::ut::PathHandlers handlers;

    SessionTokenServer st_responder;
    handlers["/api/user/get_session_token.php"] = PHBind(&st_responder);

    FolderGetContentServer rcc_responder;
    handlers["/api/1.0/folder/get_content.php"] = PHBind(&rcc_responder);

    api::ut::SessionTokenTestServer server(
        &io_service,
        address,
        port,
        std::move(handlers)
    );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);
    http_config->AllowSelfSignedCertificate();

    api::SessionMaintainer stm(
        http_config,
        host
    );
    stm.SetLoginCredentials( api::credentials::Email{ username, password } );

    api::folder::get_content::Request get_content(
        "myfiles",  // folder_key
        0,  // chunk
        api::folder::get_content::ContentType::Folders  // content_type
    );

    stm.Call(
        get_content,
        [&]( const api::folder::get_content::Response & response )
        {
            ft.Stop();
            io_service.stop();

            if ( response.error_code )
            {
                std::cout << response.debug << std::endl;

                std::ostringstream ss;
                ss << "Error: " << response.error_string << std::endl;
                BOOST_FAIL(ss.str());
            }
            else
            {
                std::cout << "Success!" << std::endl;
            }
        }
    );

    std::cout << "Starting " << TestName() <<  " main loop" << std::endl;
    io_service.run();

    BOOST_CHECK_EQUAL( st_responder.tokens_served(), 1 );
}

BOOST_AUTO_TEST_CASE(FolderGetContentFiles)
{
    boost::asio::io_service io_service;
    FailTimer ft(&io_service, 2);

    api::ut::PathHandlers handlers;

    SessionTokenServer st_responder;
    handlers["/api/user/get_session_token.php"] = PHBind(&st_responder);

    FolderGetContentServer rcc_responder;
    handlers["/api/1.0/folder/get_content.php"] = PHBind(&rcc_responder);

    api::ut::SessionTokenTestServer server(
        &io_service,
        address,
        port,
        std::move(handlers)
    );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);
    http_config->AllowSelfSignedCertificate();

    api::SessionMaintainer stm(
        http_config,
        host
    );
    stm.SetLoginCredentials( api::credentials::Email{ username, password } );

    api::folder::get_content::Request get_content(
        "myfiles",  // folder_key
        0,  // chunk
        api::folder::get_content::ContentType::Files  // content_type
    );

    stm.Call(
        get_content,
        [&](const api::folder::get_content::Response & response)
        {
            ft.Stop();
            io_service.stop();

            if (response.error_code)
            {
                std::cout << response.debug << std::endl;

                std::ostringstream ss;
                ss << "Error: " << response.error_string << std::endl;
                BOOST_FAIL(ss.str());
            }
            else
            {
                std::cout << "Success!" << std::endl;
            }
        }
    );

    std::cout << "Starting " << TestName() <<  " main loop" << std::endl;
    io_service.run();

    BOOST_CHECK_EQUAL( st_responder.tokens_served(), 1 );
}

BOOST_AUTO_TEST_CASE(RepeatedTokenVerify)
{
    boost::asio::io_service io_service;

    // No Stop needed as io_service::stop called.
    FailTimer ft(&io_service, 10);

    api::ut::PathHandlers handlers;

    SessionTokenServer st_responder;
    handlers["/api/user/get_session_token.php"] = PHBind(&st_responder);

    FolderGetContentServer rcc_responder;
    handlers["/api/1.0/folder/get_content.php"] = PHBind(&rcc_responder);

    api::ut::SessionTokenTestServer server(
        &io_service,
        address,
        port,
        std::move(handlers)
    );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);
    http_config->AllowSelfSignedCertificate();

    api::SessionMaintainer stm(
        http_config,
        host
    );
    stm.SetLoginCredentials( api::credentials::Email{ username, password } );

    class RepeatCaller
    {
    public:
        RepeatCaller(
                api::SessionMaintainer * stm,
                int * times_to_run_countdown,
                boost::asio::io_service * io_service
            ) :
            stm_(stm),
            times_to_run_countdown_(times_to_run_countdown),
            io_service_(io_service),
            get_content_(
                "myfiles",  // folder_key
                0,  // chunk
                api::folder::get_content::ContentType::Files  // content_type
            )
        {
        }

        void operator()( const api::folder::get_content::Response & response )
        {
            if ( response.error_code )
            {
                std::cout << response.debug << std::endl;

                std::ostringstream ss;
                ss << "Error: " << response.error_string << std::endl;
                BOOST_FAIL(ss.str());
            }

            if ( ! *times_to_run_countdown_ )
            {
                io_service_->stop();
            }
            else
            {
                --*times_to_run_countdown_;
                Run();
            }
        }

        void Run()
        {
            stm_->Call( get_content_, *this );
        }
    private:
        api::SessionMaintainer * stm_;
        int * times_to_run_countdown_;
        boost::asio::io_service * io_service_;
        api::folder::get_content::Request get_content_;
    };

    int times_to_run_countdown = 100;
    RepeatCaller rc(&stm, &times_to_run_countdown, &io_service);

    rc.Run();

    std::cout << "Starting " << TestName() <<  " main loop" << std::endl;
    io_service.run();

    // The number of tokens requested should still be 1, as the session token
    // has no reason to expire.
    BOOST_CHECK_EQUAL( st_responder.tokens_served(), 1 );
}

BOOST_AUTO_TEST_CASE(RepeatedParallelTokenVerify)
{
    std::cout << "Begin:  " << TestName() << std::endl;

    boost::asio::io_service io_service;

    // No Stop needed as io_service::stop called.
    FailTimer ft(&io_service, 10);

    api::ut::PathHandlers handlers;

    SessionTokenServer st_responder;
    handlers["/api/user/get_session_token.php"] = PHBind(&st_responder);

    FolderGetContentServer rcc_responder;
    handlers["/api/1.0/folder/get_content.php"] = PHBind(&rcc_responder);

    api::ut::SessionTokenTestServer server(
        &io_service,
        address,
        port,
        std::move(handlers)
    );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);
    http_config->AllowSelfSignedCertificate();

    api::SessionMaintainer stm(
        http_config,
        host
    );
    stm.SetLoginCredentials( api::credentials::Email{ username, password } );

    class RepeatCaller
    {
    public:
        RepeatCaller(
                api::SessionMaintainer * stm,
                int * times_to_run_countdown,
                boost::asio::io_service * io_service
            ) :
            stm_(stm),
            times_to_run_countdown_(times_to_run_countdown),
            io_service_(io_service),
            get_content_(
                "myfiles",  // folder_key
                0,  // chunk
                api::folder::get_content::ContentType::Files  // content_type
            )
        {
        }

        void operator()( const api::folder::get_content::Response & response )
        {
            if ( response.error_code )
            {
                std::cout << response.debug << std::endl;

                std::ostringstream ss;
                ss << "Error: " << response.error_string << std::endl;
                BOOST_FAIL(ss.str());
            }

            if ( ! *times_to_run_countdown_ )
                io_service_->stop();
            else
            {
                --*times_to_run_countdown_;
                Run();
            }
        }

        void Run()
        {
            stm_->Call( get_content_, *this );
        }
    private:
        api::SessionMaintainer * stm_;
        int * times_to_run_countdown_;
        boost::asio::io_service * io_service_;
        api::folder::get_content::Request get_content_;
    };

    {
        const int count = 10;
        const std::size_t parallel_instances =
            api::SessionMaintainer::max_tokens + 4;
        std::array<int, parallel_instances> counts;
        std::array<std::unique_ptr<RepeatCaller>, parallel_instances> callers;

        for ( std::size_t i = 0; i < parallel_instances; ++i )
        {
            counts[i] = count;
            RepeatCaller * rc = new RepeatCaller(&stm, &counts[i], &io_service);
            callers[i].reset(rc);
            callers[i]->Run();
        }

        std::cout << "Starting " << TestName() <<  " main loop" << std::endl;
        io_service.run();
        std::cout << "Completed " << TestName() <<  " main loop" << std::endl;
    }

    // We shouldn't go over the number of max tokens.
    BOOST_CHECK_EQUAL(
        st_responder.tokens_served(),
        api::SessionMaintainer::max_tokens );
}

BOOST_AUTO_TEST_CASE(GetActionToken)
{
    boost::asio::io_service io_service;
    FailTimer ft(&io_service, 2);

    api::ut::PathHandlers handlers;

    SessionTokenServer st_responder;
    handlers["/api/user/get_session_token.php"] = PHBind(&st_responder);

    GetActionTokenServer at_responder;
    handlers["/api/1.0/user/get_action_token.php"] = PHBind(&at_responder);

    api::ut::SessionTokenTestServer server(
        &io_service,
        address,
        port,
        std::move(handlers)
    );

    auto http_config = mf::http::HttpConfig::Create();
    http_config->SetWorkIoService(&io_service);
    http_config->AllowSelfSignedCertificate();

    api::SessionMaintainer stm(
        http_config,
        host
    );
    stm.SetLoginCredentials( api::credentials::Email{ username, password } );

    using api::user::get_action_token::Type;

    api::user::get_action_token::Request image_request(Type::Image);
    api::user::get_action_token::Request upload_request(Type::Upload);

    int callback_count = 0;
    const int callback_count_expected = 2;
    auto callback(
        [&](const api::user::get_action_token::Response & response)
        {
            ++callback_count;

            if ( callback_count == callback_count_expected )
            {
                ft.Stop();
                io_service.stop();
            }

            if (response.error_code)
            {
                std::cout << response.debug << std::endl;

                std::ostringstream ss;
                ss << "Error: " << response.error_string << std::endl;
                BOOST_FAIL(ss.str());
            }
            else
            {
                std::cout << "Success!" << std::endl;
            }
        });

    stm.Call( image_request, callback );
    stm.Call( upload_request, callback );

    std::cout << "Starting " << TestName() <<  " main loop" << std::endl;
    io_service.run();
}

/** @todo hjones: Add test to ensure session token manager doesn't spam server
 * when failures occur. */

