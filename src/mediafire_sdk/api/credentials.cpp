/**
 * @file credentials.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "credentials.hpp"

#include <sstream>

#include "boost/variant/apply_visitor.hpp"

namespace mf {
namespace api {
namespace credentials {
bool operator==(const Email& lhs, const Email& rhs)
{
    return std::tie(lhs.email, lhs.password)
        == std::tie(rhs.email, rhs.password);
}

bool operator==(const Facebook& lhs, const Facebook& rhs)
{
    return (lhs.fb_access_token == rhs.fb_access_token);
}
}  // namespace credentials

std::size_t CredentialsHash(Credentials credentials)
{
    class Visitor : public boost::static_visitor<std::size_t>
    {
    public:
        std::size_t operator()(credentials::Email email_credentials) const
        {
            std::ostringstream ss;
            ss << email_credentials.email << "::" << email_credentials.password;
            return std::hash<std::string>()(ss.str());
        }

        std::size_t operator()(credentials::Facebook facebook_credentials) const
        {
            std::ostringstream ss;
            ss << facebook_credentials.fb_access_token;
            return std::hash<std::string>()(ss.str());
        }
    };

    return boost::apply_visitor(Visitor(), credentials);
}

}  // namespace api
}  // namespace mf
