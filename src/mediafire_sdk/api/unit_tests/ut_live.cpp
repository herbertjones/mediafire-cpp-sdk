/**
 * @file ut_session_maintainer.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */

#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>

// Avoid excessive debug messages by including headers we don't want debug from
// before setting OUTPUT_DEBUG
#include "mediafire_sdk/api/session_maintainer.hpp"

#ifndef OUTPUT_DEBUG
#  define OUTPUT_DEBUG
#endif

#include "mediafire_sdk/api/billing/get_products.hpp"
#include "mediafire_sdk/api/billing/get_plans.hpp"

#include "mediafire_sdk/api/device/get_changes.hpp"
#include "mediafire_sdk/api/device/get_status.hpp"

#include "mediafire_sdk/api/file/copy.hpp"
#include "mediafire_sdk/api/file/create.hpp"
#include "mediafire_sdk/api/file/file_delete.hpp"
#include "mediafire_sdk/api/file/get_info.hpp"
#include "mediafire_sdk/api/file/get_links.hpp"
#include "mediafire_sdk/api/file/move.hpp"
#include "mediafire_sdk/api/file/update.hpp"

#include "mediafire_sdk/api/folder/copy.hpp"
#include "mediafire_sdk/api/folder/create.hpp"
#include "mediafire_sdk/api/folder/folder_delete.hpp"
#include "mediafire_sdk/api/folder/get_content.hpp"
#include "mediafire_sdk/api/folder/get_info.hpp"
#include "mediafire_sdk/api/folder/move.hpp"
#include "mediafire_sdk/api/folder/update.hpp"

#include "mediafire_sdk/api/notifications/get_cache.hpp"
#include "mediafire_sdk/api/notifications/peek_cache.hpp"

#include "mediafire_sdk/api/system/get_limits.hpp"
#include "mediafire_sdk/api/system/get_status.hpp"

#include "mediafire_sdk/api/user/get_action_token.hpp"
#include "mediafire_sdk/api/user/get_info.hpp"
#include "mediafire_sdk/api/user/get_limits.hpp"
#include "mediafire_sdk/api/user/get_login_token.hpp"
#include "mediafire_sdk/api/user/get_session_token.hpp"

#include "mediafire_sdk/utils/md5_hasher.hpp"
#include "mediafire_sdk/utils/string.hpp"

#include "boost/algorithm/string/find.hpp"
#include "boost/algorithm/string/join.hpp"

#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int_distribution.hpp"

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#  include "boost/asio/impl/src.hpp"  // Define once in program
#  include "boost/asio/ssl/impl/src.hpp"  // Define once in program
#endif

#include "boost/format.hpp"

#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "mediafire_sdk/api/unit_tests/session_token_test_server.hpp"
#include "mediafire_sdk/api/unit_tests/api_ut_helpers.hpp"

#define BOOST_TEST_MODULE ApiLive
#include "boost/test/unit_test.hpp"

namespace posix_time = boost::posix_time;
namespace api = mf::api;

#if ! defined(TEST_USER_1_USERNAME) || ! defined(TEST_USER_1_PASSWORD) \
 || ! defined(TEST_USER_2_USERNAME) || ! defined(TEST_USER_2_PASSWORD)
# error "TEST_USER defines not set."
#endif

using TokenContainer = std::vector<mf::api::SessionTokenData>;

namespace constants {
const std::string host = "www.mediafire.com";
}  // namespace constants
namespace user1 {
const std::string username = TEST_USER_1_USERNAME;
const std::string password = TEST_USER_1_PASSWORD;
}  // namespace user1
namespace user2 {
const std::string username = TEST_USER_2_USERNAME;
const std::string password = TEST_USER_2_PASSWORD;
}  // namespace user2

namespace globals {

std::string test_folderkey;
std::string test_folder_name;

std::string test_folderkey2;
std::string test_folder_name2;

std::string test_folderkey3;
std::string test_folder_name3;

std::string test_quickkey;
std::string test_file_name;

std::string test_quickkey2;
std::string test_file_name2;

std::string foreign_folderkey;
std::string foreign_folder_name;

uint32_t known_revision = 0;
bool has_updated_files = false;
bool has_updated_folders = false;
bool has_deleted_files = false;
bool has_deleted_folders = false;
}  // namespace globals

namespace {
std::string TestName()
{
    return boost::unit_test::framework::current_test_case().p_name;
}

struct LoginCredentials
{
    std::string username;
    std::string password;
};
}  // namespace

namespace globals {
const api::credentials::Email connection1{ user1::username, user1::password };
const api::credentials::Email connection2{ user2::username, user2::password };
}  // namespace globals

namespace {
class Fixture
{
public:
    Fixture() :
        credentials_(globals::connection1),
        http_config_(mf::http::HttpConfig::Create()),
        timeout_timer_(*http_config_->GetWorkIoService()),
        stm_(http_config_, constants::host)
    {
        stm_.SetLoginCredentials( credentials_ );
    }

    ~Fixture()
    {
        http_config_->StopService();
    }

    void SetUser2()
    {
        credentials_ = globals::connection2;
    }

    void Start()
    {
        assert(http_config_);

        http_config_->GetWorkIoService()->reset();
        http_config_->RunService();
    }

    void StartWithDefaultTimeout()
    {
        StartWithTimeout( posix_time::seconds(15) );
    }

    void StartWithTimeout(
            boost::posix_time::time_duration timeout_length
        )
    {
        timeout_timer_.expires_from_now(timeout_length);

        timeout_timer_.async_wait(
            boost::bind(
                &Fixture::HandleTimeout,
                this,
                boost::asio::placeholders::error
            )
        );

        Start();
    }

    void Stop()
    {
        timeout_timer_.cancel();

        // Stop any ongoing timeouts so that io_service::run stops on its own
        stm_.StopTimeouts();
    }

    template<typename Request, typename Callback>
    void Call(Request request, Callback callback)
    {
        requests_.insert( stm_.Call( request, callback ) );
    }

    template<typename Response>
    void Fail(const Response & response)
    {
        std::cout << response.debug << std::endl;

        if (response.error_string)
            Fail(response.error_string);
        else
            Fail(response.error_code.message());
    }

    void Fail(const boost::optional<std::string> & maybe_str)
    {
        if (maybe_str)
            Fail(*maybe_str);
        else
            Fail(std::string("Unknown error"));
    }

    void Fail(const std::string & str)
    {
        Stop();

        std::cout << TestName() << ": Failure" << std::endl;
        BOOST_FAIL(str);
    }

    void Fail(const char* errorString)
    {
        Fail(std::string(errorString));
    }

    void Success()
    {
        Stop();

        std::cout << TestName() << ": Success" << std::endl;
    }

    void ChangeCredentials(const api::Credentials& credentials)
    {
        stm_.SetLoginCredentials(credentials);
    }

    void Debug(const boost::property_tree::wptree & pt)
    {
        std::wostringstream ss;
        boost::property_tree::write_json( ss, pt );
        std::cout << TestName() << ": JSON received:\n"
            << mf::utils::wide_to_bytes(ss.str()) << std::endl;
    }

    void Debug(const std::string & str)
    {
        std::ostringstream ss;
        std::cout << TestName() << ": " << str << std::endl;
    }

protected:
    void HandleTimeout(const boost::system::error_code & err)
    {
        if (!err)
        {
            std::ostringstream ss;
            ss << "Timeout failure in " << TestName() << std::endl;
            BOOST_FAIL(ss.str());

            // Stop not safe in this instance. Cancel all requests instead.
            for ( const api::SessionMaintainer::Request & request : requests_ )
                request->Cancel();
        }
    }

    api::Credentials credentials_;
    mf::http::HttpConfig::Pointer http_config_;
    boost::asio::deadline_timer timeout_timer_;
    api::SessionMaintainer stm_;
    std::set<api::SessionMaintainer::Request> requests_;
};
std::string RandomAlphaNum(int length)
{
    std::string ret;

    // No hex chars here to avoid accidental success if chunking is broken.
    static std::string chars(
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        );

    static boost::random::random_device rng;
    static boost::random::uniform_int_distribution<> index_dist(
        0, chars.size() - 1);

    ret.reserve(length);

    for (int i = 0; i < length; ++i)
        ret.push_back( chars[index_dist(rng)] );

    return ret;
}

}  // namespace

BOOST_FIXTURE_TEST_SUITE( s, Fixture )

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

BOOST_AUTO_TEST_CASE(SystemGetLimits)
{
    Call(
        api::system::get_limits::Request(),
        [&](const api::system::get_limits::Response & response)
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

BOOST_AUTO_TEST_CASE(SystemGetStatus)
{
    Call(
        api::system::get_status::Request(),
        [&](const api::system::get_status::Response & response)
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

BOOST_AUTO_TEST_CASE(FolderGetContentLive)
{
    api::folder::get_content::Request get_content(
        "myfiles",  // folder_key
        0,  // chunk
        api::folder::get_content::ContentType::Files  // content_type
    );

    Call(
        get_content,
        [&](const api::folder::get_content::Response & response)
        {
            Stop();

            if ( response.error_code )
            {
                std::ostringstream ss;
                ss << "Error: " << response.error_string << std::endl;
                BOOST_FAIL(ss.str());
            }
            else
                std::cout << "Success!" << std::endl;
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(GetActionTokenLive)
{
    api::user::get_action_token::Request request(
        api::user::get_action_token::Type::Image);
    Call(
        request,
        [&](const api::user::get_action_token::Response & response)
        {
            if ( response.error_code )
                Fail(response);
            else
                Success();
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(GetBillingProducts)
{
    api::billing::get_products::Request request;
    request.SetActive(api::billing::get_products::Activity::Active);

    Call(
        request,
        [&](const api::billing::get_products::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                BOOST_CHECK( ! response.products.empty() );

                for ( const api::billing::get_products::Response::Product & it
                    : response.products )
                {
                    BOOST_CHECK( ! it.description.empty() );
                }
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(GetBillingPlans)
{
    Call(
        api::billing::get_plans::Request(),
        [&](const api::billing::get_plans::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                BOOST_CHECK( ! response.plans.empty() );

                for ( const api::billing::get_plans::Response::Plan & it
                    : response.plans )
                {
                    BOOST_CHECK( ! it.description.empty() );
                }
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(CreateFolder)
{
    globals::test_folder_name = RandomAlphaNum(20);

    Debug( globals::test_folder_name );

    Call(
        api::folder::create::Request(
            globals::test_folder_name
            ),
        [&](const api::folder::create::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                globals::test_folderkey = response.folderkey;

                BOOST_CHECK( ! response.folderkey.empty() );
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(CreateFolder2)
{
    globals::test_folder_name2 = RandomAlphaNum(20);

    Debug( globals::test_folder_name2 );

    Call(
        api::folder::create::Request(
            globals::test_folder_name2
            ),
        [&](const api::folder::create::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                globals::test_folderkey2 = response.folderkey;

                BOOST_CHECK( ! response.folderkey.empty() );
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FolderGetInfoMyFilesBlank)
{
    Call(
        api::folder::get_info::Request(""),
        [&](const api::folder::get_info::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                BOOST_CHECK( ! response.folderkey.empty() );
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FolderGetInfoMyFilesExplicit)
{
    Call(
        api::folder::get_info::Request("myfiles"),
        [&](const api::folder::get_info::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                BOOST_CHECK( ! response.folderkey.empty() );
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FolderGetInfo)
{
    Call(
        api::folder::get_info::Request(
            globals::test_folderkey
            ),
        [&](const api::folder::get_info::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                BOOST_CHECK( ! response.folderkey.empty() );
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(CreateFile)
{
    // Only .txt files allowed currently.
    globals::test_file_name = RandomAlphaNum(20) + ".txt";

    Debug( globals::test_file_name );

    api::file::create::Request request;

    request.SetFilename(globals::test_file_name);
    request.SetParentFolderkey(globals::test_folderkey);

    Call(
        request,
        [&](const api::file::create::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                globals::test_quickkey = response.quickkey;

                BOOST_CHECK( ! response.quickkey.empty() );
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(CopyFile)
{
    api::file::copy::Request request(globals::test_quickkey);

    request.SetTargetParentFolderkey(globals::test_folderkey);

    Call(
        request,
        [&](const api::file::copy::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                globals::test_quickkey2 = response.quickkey;

                Debug( response.quickkey );

                BOOST_CHECK( ! response.quickkey.empty() );
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FileGetInfo)
{
    Call(
        api::file::get_info::Request(globals::test_quickkey2),
        [&](const api::file::get_info::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                globals::test_file_name2 = response.filename;

                Debug( globals::test_file_name2 );

                BOOST_CHECK( ! response.quickkey.empty() );
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FileMove)
{
    // Move to root
    api::file::move::Request request(globals::test_quickkey2, "");

    Call(
        request,
        [&](const api::file::move::Response & response)
        {
            if ( response.error_code )
                Fail(response);
            else
                Success();
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FileRename)
{
    api::file::update::Request request(globals::test_quickkey2);

    std::string new_name(RandomAlphaNum(20));

    request.SetFilename(new_name);

    Call(
        request,
        [&](const api::file::update::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();
                globals::test_file_name2 = new_name;
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FileMakePrivate)
{
    api::file::update::Request request(globals::test_quickkey2);

    request.SetPrivacy( api::file::update::Privacy::Private );

    Call(
        request,
        [&](const api::file::update::Response & response)
        {
            if ( response.error_code )
                Fail(response);
            else
                Success();
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FileIsPrivate)
{
    Call(
        api::file::get_info::Request(globals::test_quickkey2),
        [&](const api::file::get_info::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                BOOST_CHECK(
                    response.privacy == api::file::get_info::Privacy::Private
                );
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(DeleteFile)
{
    Call(
        api::file::file_delete::Request(globals::test_quickkey2),
        [&](const api::file::file_delete::Response & response)
        {
            if ( response.error_code )
                Fail(response);
            else
                Success();
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(FileGetLinks)
{
    std::vector<std::string> quickkeys = {globals::test_quickkey};
    Call(
        api::file::get_links::Request(quickkeys),
        [&](const api::file::get_links::Response & response)
        {
            if ( response.error_code )
                Fail(response);
            else
                Success();
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(CopyFolder)
{
    Call(
        api::folder::copy::Request(
            globals::test_folderkey,
            globals::test_folderkey2
            ),
        [&](const api::folder::copy::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                globals::test_folderkey3 = response.folderkey;
                globals::test_folder_name3 = globals::test_folder_name;
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(MoveFolder)
{
    Call(
        api::folder::move::Request(
            globals::test_folderkey3,
            globals::test_folderkey
            ),
        [&](const api::folder::move::Response & response)
        {
            if ( response.error_code )
                Fail(response);
            else
                Success();
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(RenameFolder)
{
    api::folder::update::Request request( globals::test_folderkey3 );

    request.SetFoldername("CopyFolder");

    Call(
        request,
        [&](const api::folder::update::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();
                globals::known_revision = response.device_revision;
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(ConfirmCopyMove)
{
    api::folder::get_info::Request request( globals::test_folderkey );

    request.SetDetails( api::folder::get_info::Details::FullDetails );

    Call(
        request,
        [&](const api::folder::get_info::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                BOOST_CHECK(
                    response.total_files && *response.total_files == 2
                    && response.total_folders && *response.total_folders == 1
                    );
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(NotificationPeekCache)
{
    Call(
        api::notifications::peek_cache::Request(),
        [&](const api::notifications::peek_cache::Response & response)
        {
            if ( response.error_code )
                Fail(response);
            else
                Success();
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(NotificationGetCache)
{
    Call(
        api::notifications::get_cache::Request(),
        [&](const api::notifications::get_cache::Response & response)
        {
            if ( response.error_code )
                Fail(response);
            else
                Success();
        });

    StartWithDefaultTimeout();
}

#if 0 // API delete is asynchronous so next operations fail if we do this.
BOOST_AUTO_TEST_CASE(FolderDelete2)
{
    Call(
        api::folder::folder_delete::Request(
            globals::test_folderkey2
            ),
        [&](const api::folder::folder_delete::Response & response)
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
#endif

BOOST_AUTO_TEST_CASE(DeviceGetChanges)
{
    uint32_t revision = 0;
    if ( globals::known_revision > 500 )
        revision = globals::known_revision - 500;
    revision -= revision % 500;

    std::ostringstream ss;
    ss << "Current revision: " << globals::known_revision;
    ss << " Checking revision: " << revision;
    Debug(ss.str());

    Call(
        api::device::get_changes::Request(revision),
        [&](const api::device::get_changes::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                globals::has_updated_files   = ! response.updated_files.empty();
                globals::has_updated_folders = ! response.updated_folders.empty();
                globals::has_deleted_files   = ! response.deleted_files.empty();
                globals::has_deleted_folders = ! response.deleted_folders.empty();
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(DeviceGetChanges2)
{
    uint32_t revision = 0;
    if (globals::known_revision > 500)
        revision = globals::known_revision - 500;
    revision += (500 - revision % 500);

    std::ostringstream ss;
    ss << "Current revision: " << globals::known_revision;
    ss << " Checking revision: " << revision;
    Debug(ss.str());

    Call(
        api::device::get_changes::Request(revision),
        [&](const api::device::get_changes::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                globals::has_updated_files   |= ! response.updated_files.empty();
                globals::has_updated_folders |= ! response.updated_folders.empty();
                globals::has_deleted_files   |= ! response.deleted_files.empty();
                globals::has_deleted_folders |= ! response.deleted_folders.empty();

                // Files were created.
                BOOST_CHECK( globals::has_updated_files );

                // Folders were created.
                BOOST_CHECK( globals::has_updated_folders );

                // We have deleted files.
                BOOST_CHECK( globals::has_deleted_files );

                // We have deleted a folder.
                // BOOST_CHECK( globals::has_deleted_folders );
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(CreateFolderUser2)
{
    SetUser2();

    globals::foreign_folder_name = RandomAlphaNum(20);

    Debug( globals::foreign_folder_name );

    Call(
        api::folder::create::Request(
            globals::foreign_folder_name
            ),
        [&](const api::folder::create::Response & response)
        {
            if ( response.error_code )
            {
                Fail(response);
            }
            else
            {
                Success();

                globals::foreign_folderkey = response.folderkey;

                BOOST_CHECK( ! response.folderkey.empty() );
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(DeviceGetStatusPreDelete1)
{
    Call(
        api::device::get_status::Request(),
        [&](const api::device::get_status::Response & response)
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

BOOST_AUTO_TEST_CASE(FolderDelete1)
{
    std::vector<std::string> folderkeys;

    std::function<void(const std::string &)> add_folderkey(
        [&](const std::string & folderkey)
        {
            if (!folderkey.empty())
                folderkeys.push_back(folderkey);
        });
    add_folderkey(globals::test_folderkey);
    // add_folderkey(globals::test_folderkey2);
    // add_folderkey(globals::foreign_folderkey);

    if (!folderkeys.empty())
    {
        std::string keys( boost::join(folderkeys, ",") );

        std::cout << "Deleting keys: " << keys << std::endl;

        Call(
            api::folder::folder_delete::Request( keys ),
            [&](const api::folder::folder_delete::Response & response)
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
}

BOOST_AUTO_TEST_CASE(DeviceGetStatusPreDelete2)
{
    Call(
        api::device::get_status::Request(),
        [&](const api::device::get_status::Response & response)
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

BOOST_AUTO_TEST_CASE(FolderDelete2)
{
    std::vector<std::string> folderkeys;

    std::function<void(const std::string &)> add_folderkey(
        [&](const std::string & folderkey)
        {
            if (!folderkey.empty())
                folderkeys.push_back(folderkey);
        });
    // add_folderkey(globals::test_folderkey);
    add_folderkey(globals::test_folderkey2);
    // add_folderkey(globals::foreign_folderkey);

    if (!folderkeys.empty())
    {
        std::string keys( boost::join(folderkeys, ",") );

        std::cout << "Deleting keys: " << keys << std::endl;

        Call(
            api::folder::folder_delete::Request( keys ),
            [&](const api::folder::folder_delete::Response & response)
            {
                if ( response.error_code )
                    Fail(response);
                else
                    Success();
            });

        StartWithDefaultTimeout();
    }
}

BOOST_AUTO_TEST_CASE(DeviceGetStatusPostDelete)
{
    Call(
        api::device::get_status::Request(),
        [&](const api::device::get_status::Response & response)
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

BOOST_AUTO_TEST_CASE(GetLoginToken)
{
    api::credentials::Email email_creds = {user1::username, user1::password};
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
                    << "https://" << constants::host
                    << "/api/user/login_with_token.php?login_token="
                    << response.login_token << std::endl;
            }
        });

    StartWithDefaultTimeout();
}

BOOST_AUTO_TEST_CASE(EkeyTest)
{
    api::credentials::Email email_creds = {user1::username, user1::password};
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
                ekey_creds = api::credentials::Ekey{response.ekey, user1::password};
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

// Spam the server with the same call. Was used to verify claims made by
// another team that there could be a problem with session tokens expiring after
// 100 calls or so. No problem was discovered. Place assert in
// api::SessionMaintainer::HandleSessionTokenFailure to test again.
BOOST_AUTO_TEST_CASE(SpamTest)
{
    const uint32_t requests_to_be_made = 50;

    uint32_t times_failed = 0;
    uint32_t times_returned = 0;

    std::function<void(const api::device::get_changes::Response &,uint32_t)>
        callback(
        [this, &times_returned, &times_failed, requests_to_be_made](
                const api::device::get_changes::Response & response,
                uint32_t count
            )
        {
            std::cout << "SpamTest: Count: " << count << std::endl;

            if ( response.error_code )
            {
                ++times_failed;
                std::cout << "Error: " << response.error_string << std::endl;
            }

            ++times_returned;
            if (times_returned == requests_to_be_made)
            {
                if (times_failed)
                    Fail(response);
                else
                    Success();
            }
        });

    for ( uint32_t i = 1; i <= requests_to_be_made; ++i )
    {
        Call(
                api::device::get_changes::Request(0),
                std::bind(callback, std::placeholders::_1, i)
            );
    }

    Start();

    BOOST_CHECK_EQUAL( times_failed, 0 );
    BOOST_CHECK_EQUAL( times_returned, requests_to_be_made );
}

BOOST_AUTO_TEST_SUITE_END()
