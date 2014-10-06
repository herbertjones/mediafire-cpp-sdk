/**
 * @file credentials.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "credentials.hpp"

#include <sstream>

#include "boost/variant/apply_visitor.hpp"
#include "boost/tuple/tuple_comparison.hpp"

namespace tie_ns = boost;

namespace mf {
namespace api {
namespace credentials {
bool operator==(const Email& lhs, const Email& rhs)
{
    return tie_ns::tie(lhs.email, lhs.password)
        == tie_ns::tie(rhs.email, rhs.password);
}

bool operator==(const Ekey& lhs, const Ekey& rhs)
{
    return tie_ns::tie(lhs.ekey, lhs.password)
        == tie_ns::tie(rhs.ekey, rhs.password);
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

        std::size_t operator()(credentials::Ekey ekey_credentials) const
        {
            std::ostringstream ss;
            ss << ekey_credentials.ekey << "::" << ekey_credentials.password;
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
