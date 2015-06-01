/**
 * @file src/mediafire_sdk/api/unit_tests/ut_live.hpp
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

#include "boost/asio.hpp"
#include "boost/asio/ssl.hpp"

#if !defined(TEST_USER_1_USERNAME) || !defined(TEST_USER_1_PASSWORD)           \
        || !defined(TEST_USER_2_USERNAME) || !defined(TEST_USER_2_PASSWORD)
#error "TEST_USER defines not set."
#endif

namespace ut
{

namespace constants
{
extern const std::string host;
}  // namespace constants

namespace user1
{
extern const std::string username;
extern const std::string password;
}  // namespace user1

namespace user2
{
extern const std::string username;
extern const std::string password;
}  // namespace user2

namespace globals
{
extern const mf::api::credentials::Email connection1;
extern const mf::api::credentials::Email connection2;
}  // namespace globals

class Fixture
{
public:
    Fixture();
    ~Fixture();
    void SetUser2();
    void Start();
    void StartWithDefaultTimeout();
    void StartWithTimeout(boost::posix_time::time_duration timeout_length);
    void Stop();

    template <typename Request, typename Callback>
    void Call(Request request, Callback callback)
    {
        requests_.insert(stm_.Call(request, callback));
    }

    template <typename Request, typename Callback>
    void CallIn(boost::posix_time::time_duration start_in,
                Request request,
                Callback callback)
    {
        call_in_timer_.expires_from_now(start_in);

        call_in_timer_.async_wait(
                [this, request, callback](const boost::system::error_code & err)
                {
                    if (!err)
                        Call(request, callback);
                    else
                        Fail("CallIn timer cancelled.");
                });
    }

    template <typename Response>
    void Fail(const Response & response)
    {
        std::cout << response.debug << std::endl;

        if (response.error_string)
            Fail(response.error_string);
        else
            Fail(response.error_code.message());
    }

    void Fail(const boost::optional<std::string> & maybe_str);
    void Fail(const std::string & str);
    void Fail(const char * errorString);

    void Success();

    void ChangeCredentials(const mf::api::Credentials & credentials);

    void Debug(const boost::property_tree::wptree & pt);

    void Debug(const std::string & str);

    void WaitForAnyAsyncOperationsToComplete();

    template <typename Head, typename... Tail>
    inline void Log(const Head head, Tail &&... tail)
    {
        std::ostringstream ss;
        ss << head;
        LogStep(ss, tail...);
        Debug(ss.str());
    }

protected:
    void HandleTimeout(const boost::system::error_code & err);

    inline void LogStep(std::ostringstream &)
    {
        // Base case.  Nothing to do.
    }

    template <typename Head, typename... Tail>
    inline void LogStep(std::ostringstream & ss,
                        const Head head,
                        Tail &&... tail)
    {
        ss << ' ' << head;
        LogStep(ss, tail...);
    }

    void HandleConnectionStateChange(
            mf::api::ConnectionState new_connection_state);

    void HandleSessionStateChange(mf::api::SessionState new_session_state);

    mf::api::Credentials credentials_;
    mf::http::HttpConfig::Pointer http_config_;
    boost::asio::deadline_timer timeout_timer_;
    boost::asio::deadline_timer call_in_timer_;
    mf::api::SessionMaintainer stm_;
    std::set<mf::api::SessionMaintainer::Request> requests_;
    bool async_wait_logged_;
};

std::string RandomAlphaNum(int length);

std::string TestName();

}  // namespace ut
