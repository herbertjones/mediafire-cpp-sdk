/**
 * @file api/user/update.hpp
 * @brief API request: /api/1.3/user/update
 *
 * @copyright Copyright 2014 Mediafire
 *
 * This file was generated by gen_api_template.py. Do NOT edit by hand.
 */
#pragma once

#include <string>
#include <vector>

#include "mediafire_sdk/http/shared_buffer.hpp"
#include "mediafire_sdk/http/headers.hpp"
#include "mediafire_sdk/api/response_base.hpp"

#include "boost/date_time/posix_time/ptime.hpp"

namespace mf {
namespace api {
/** API action path "user" */
namespace user {
/** API action "user/update" */
namespace update {
/** API path "/api/1.3/user/update" */
namespace v1_3 {

enum class Gender
{
    /** API value "male" */
    Male,
    /** API value "female" */
    Female,
    /** API value "none" */
    None
};

enum class ReceiveNewsletter
{
    /** API value "no" */
    No,
    /** API value "yes" */
    Yes
};

enum class PrimaryUsage
{
    /** API value "none" */
    None,
    /** API value "home" */
    Home,
    /** API value "work" */
    Work,
    /** API value "school" */
    School
};

/**
 * @class Response
 * @brief Response from API request "user/update"
 */
class Response : public ResponseBase
{
public:
};

class Impl;

/**
 * @class Request
 * @brief Make API request "user/update"
 */
class Request
{
public:

    // Enums in class namespace for usage with templates
    using Gender = enum Gender;
    using ReceiveNewsletter = enum ReceiveNewsletter;
    using PrimaryUsage = enum PrimaryUsage;

    /**
     * API request "user/update"
     */
    Request();

    /**
     * Optional API parameter "current_password"
     *
     * @param current_password The current password associated with the account.
     *                         Required if email or password are passed.
     */
    void SetCurrentPassword(std::string current_password);

    /**
     * Optional API parameter "subdomain"
     *
     * @param subdomain The name to use for the session user's account's
     *                  subdomain on MediaFire. Normally, this is the same as
     *                  company_name. Required if company_name is passed.
     */
    void SetSubdomain(std::string subdomain);

    /**
     * Optional API parameter "company_name"
     *
     * @param company_name The name of the sessions user's company. Required if
     *                     subdomain is passed.
     */
    void SetCompanyName(std::string company_name);

    /**
     * Optional API parameter "email"
     *
     * @param email An email address to be used as the user name for the
     *              account.
     */
    void SetEmail(std::string email);

    /**
     * Optional API parameter "password"
     *
     * @param password New password for the account.
     */
    void SetPassword(std::string password);

    /**
     * Optional API parameter "first_name"
     *
     * @param first_name First name of user.
     */
    void SetFirstName(std::string first_name);

    /**
     * Optional API parameter "last_name"
     *
     * @param last_name Last name of user.
     */
    void SetLastName(std::string last_name);

    /**
     * Optional API parameter "display_name"
     *
     * @param display_name The name that will be displayed to the public.
     */
    void SetDisplayName(std::string display_name);

    /**
     * Optional API parameter "birth_date"
     *
     * @param birth_date The session user's birth date.  It should take the
     *                   format "yyyy-mm-dd".
     */
    void SetBirthDate(std::string birth_date);

    /**
     * Optional API parameter "gender"
     *
     * @param gender The session user's gender.
     */
    void SetGender(Gender gender);

    /**
     * Optional API parameter "website"
     *
     * @param website The session user's URL.
     */
    void SetWebsite(std::string website);

    /**
     * Optional API parameter "location"
     *
     * @param location The session user's address.
     */
    void SetLocation(std::string location);

    /**
     * Optional API parameter "receive_newsletter"
     *
     * @param receive_newsletter Receive MediaFire site news via email.
     */
    void SetReceiveNewsletter(ReceiveNewsletter receive_newsletter);

    /**
     * Optional API parameter "primary_usage"
     *
     * @param primary_usage The environment this MediaFire account is intended
     *                      to be used in the most.
     */
    void SetPrimaryUsage(PrimaryUsage primary_usage);

    /**
     * Optional API parameter "timezone"
     *
     * @param timezone The code of the local timezone of the session user.
     */
    void SetTimezone(std::string timezone);

    // Remaining functions are for use by API library only. --------------------

    /** Requester/SessionMaintainer expected type. */
    typedef Response ResponseType;

    /** Requester/SessionMaintainer expected type. */
    typedef std::function< void( const ResponseType & data)> CallbackType;

    /** Requester/SessionMaintainer expected type. */
    void SetCallback( CallbackType callback_function );

    /** Requester expected method. */
    void HandleContent(
            const std::string & url,
            const mf::http::Headers & headers,
            const std::string & content
        );

    /** Requester expected method. */
    void HandleError(
            const std::string & url,
            std::error_code ec,
            const std::string & error_string
        );

    /** Requester expected method. */
    std::string Url(const std::string & hostname) const;

    /** Requester optional method. */
    mf::http::SharedBuffer::Pointer GetPostData();

    /** SessionMaintainer expected method. */
    void SetSessionToken(
            std::string session_token,
            std::string time,
            int secret_key
        );
private:
    std::shared_ptr<Impl> impl_;
};
}  // namespace v1_3

}  // namespace update
}  // namespace user
}  // namespace api
}  // namespace mf
