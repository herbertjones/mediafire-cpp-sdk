/**
 * @file url.hpp
 * @author Herbert Jones
 * @brief Class for parsing urls.
 *
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>
#include <stdexcept>

namespace mf {
namespace http {

/**
 * @class InvalidUrl
 *
 * Thrown when a Url object can not be created from a URL string due to the
 * string being unparsable.
 */
struct InvalidUrl : public std::runtime_error
{
    /**
     * @brief Construct InvalidUrl.
     *
     * @param[in] what Description of error.
     */
    explicit InvalidUrl(const std::string& what) :
        std::runtime_error(what) {}
};


/**
 * @class Url
 * @brief Encapsulates and parses a URL.
 */
class Url
{
public:
    /**
     * @brief Construct a Url from a url string.
     *
     * @param[in] url URL to encapsulate and parse.
     */
    explicit Url(const std::string & url);

    /**
     * @brief The URL scheme.
     *
     * In "http://user:pass@www.host.com:8080/myfiles.php?abc=5#123" the scheme
     * is "http".
     *
     * @return The scheme.
     */
    std::string scheme() const {return scheme_;}

    /**
     * @brief The URL host.
     *
     * In "http://user:pass@www.host.com:8080/myfiles.php?abc=5#123" the host
     * is "www.host.com".
     *
     * @return The host.
     */
    std::string host() const {return host_;}

    /**
     * @brief The URL port.
     *
     * In "http://user:pass@www.host.com:8080/myfiles.php?abc=5#123" the port
     * is "8080".
     *
     * In "http://user:pass@www.host.com/myfiles.php?abc=5#123" the port
     * is "".
     *
     * @return The port if set. If not set returns an empty string..
     */
    std::string port() const {return port_;}

    /**
     * @brief The URL user.
     *
     * In "http://user:pass@www.host.com:8080/myfiles.php?abc=5#123" the user
     * is "user".
     *
     * @return The user.
     */
    std::string user() const {return user_;}

    /**
     * @brief The URL password.
     *
     * In "http://user:pass@www.host.com:8080/myfiles.php?abc=5#123" the
     * password is "pass".
     *
     * @return The password.
     */
    std::string password() const {return pass_;}

    /**
     * @brief The URL path.
     *
     * In "http://user:pass@www.host.com:8080/myfiles.php?abc=5#123" the path
     * is "/myfiles.php".
     *
     * @return The path.
     */
    std::string path() const {return path_;}

    /**
     * @brief The URL full path.
     *
     * In "http://user:pass@www.host.com:8080/myfiles.php?abc=5#123" the full
     * path is "/myfiles.php?abc#123".
     *
     * @return The full path.
     */
    std::string full_path() const {return full_path_;}

    /**
     * @brief The URL query.
     *
     * In "http://user:pass@www.host.com:8080/myfiles.php?abc=5#123" the query
     * is "abc=5".
     *
     * @return The query.
     */
    std::string query() const {return query_;}

    /**
     * @brief The URL fragment.
     *
     * In "http://user:pass@www.host.com:8080/myfiles.php?abc=5#123" the
     * fragment is "123".
     *
     * @return The fragment.
     */
    std::string fragment() const {return fragment_;}

    /**
     * @brief The original URL.
     *
     * @return The URL.
     */
    std::string url() const {return url_;}

    /**
     * Create a Url from a redirect, which may or may not contain the host and
     * protocol.
     *
     * If the host is not specified, then the Url returned contains information
     * from this Url.
     *
     * @param[in] redirect Redirect URL
     *
     * @return Usable Url
     */
    Url FromRedirect(const std::string & redirect) const;

private:
    std::string url_;
    std::string scheme_;
    std::string host_;
    std::string port_;
    std::string user_;
    std::string pass_;
    std::string path_;
    std::string full_path_;
    std::string query_;
    std::string fragment_;

    void ParsePath(std::string::const_iterator path_start);
};

}  // namespace http
}  // namespace mf
