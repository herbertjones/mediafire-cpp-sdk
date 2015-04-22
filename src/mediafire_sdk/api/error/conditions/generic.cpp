/**
 * @file generic.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "generic.hpp"

#include <cassert>
#include <sstream>
#include <string>

#include "mediafire_sdk/utils/noexcept.hpp"

namespace {
/**
 * @class ApiConditionImpl
 * @brief std::error_category implementation for api namespace
 */
class ApiConditionImpl : public std::error_category
{
public:
    /// The name of this error category.
    virtual const char* name() const NOEXCEPT;

    /// The message belonging to the error code.
    virtual std::string message(int ev) const;

    /**
     * @brief Compare other error codes to your error conditions/values, or
     *        pass them to other comparers.
     *
     * Any of your error codes that correspond to std::errc types should be
     * matched up here, and return true if so.
     *
     * See: http://en.cppreference.com/w/cpp/error/errc
     */
    virtual bool equivalent(
        const std::error_code& code,
        int condition
    ) const NOEXCEPT;
};

const char* ApiConditionImpl::name() const NOEXCEPT
{
    return "api";
}

std::string ApiConditionImpl::message(int ev) const
{
    using mf::api::errc;

    switch (static_cast<errc>(ev))
    {
        case errc::ContentInvalidFormat:
            return "content invalid format";
        case errc::ContentInvalidData:
            return "content invalid data";
        case errc::BadRequest:
            return "bad request";
        case errc::Forbidden:
            return "forbidden";
        case errc::NotFound:
            return "not found";
        case errc::InternalServerError:
            return "internal server error";
        case errc::ApiInternalServerError:
            return "api internal server error";
        case errc::UnknownApiError:
            return "unknown api error";
        case errc::SessionTokenUnavailableTimeout:
            return "session token unavailable timeout";
        case errc::ConnectionUnavailableTimeout:
            return "connection unavailable timeout";

        case errc::AccessDenied:
            return "access denied";
        case errc::AccountAlreadyLinked:
            return "account already linked";
        case errc::ApiRequestLimitExceeded:
            return "api request limit exceeded";
        case errc::ApiVersionDeprecated:
            return "api version deprecated";
        case errc::ApiVersionMissing:
            return "api version missing";
        case errc::ApiVersionRemoved:
            return "api version removed";
        case errc::ApplicationIdDisabled:
            return "application id disabled";
        case errc::ApplicationIdSuspended:
            return "application id suspended";
        case errc::ApplicationInvalid:
            return "application invalid";
        case errc::AsyncOperationInProgress:
            return "async operation in progress";
        case errc::BillingFailure:
            return "billing failure";
        case errc::BillingTransactionDuplicate:
            return "billing transaction duplicate";
        case errc::BulkFileCountExceeded:
            return "bulk file count exceeded";
        case errc::BulkFileCountZero:
            return "bulk filecount zero";
        case errc::BulkFilesizeLimitConfirmationNeeded:
            return "bulk filesize limit confirmation needed";
        case errc::BulkFilesizeLimitExceeded:
            return "bulk filesize limit exceeded";
        case errc::BulkInsufficientPremiumBandwidth:
            return "bulk insufficient premium bandwidth";
        case errc::BulkInsufficientUnifiedBandwidth:
            return "bulk insufficient unified bandwidth";
        case errc::BulkOperationIllegal:
            return "bulk operation illegal";
        case errc::BulkOperationInvalid:
            return "bulk operation invalid";
        case errc::BulkPackageFailure:
            return "bulk package failure";
        case errc::BulkRequiresBandwidthConfirmtion:
            return "bulk requires bandwidth confirmtion";
        case errc::BusinessSeatLimitation:
            return "business seat limitation";
        case errc::BusinessUpgradeFailure:
            return "business upgrade failure";
        case errc::ContactAlreadyAdded:
            return "contact already added";
        case errc::ContactInvalid:
            return "contact invalid";
        case errc::CredentialsInvalid:
            return "credentials invalid";
        case errc::CreditCardInvalid:
            return "credit card invalid";
        case errc::DateInvalid:
            return "date invalid";
        case errc::DeviceInvalid:
            return "device invalid";
        case errc::DmcaBan:
            return "dmca ban";
        case errc::DmcaInvalidRange:
            return "dmca invalid range";
        case errc::DmcaInvalidUser:
            return "dmca invalid user";
        case errc::DowngradeIllegal:
            return "downgrade illegal";
        case errc::DuplicateEntry:
            return "duplicate entry";
        case errc::EmailAlreadyInUse:
            return "email already in use";
        case errc::EmailMisformatted:
            return "email misformatted";
        case errc::EmailRejected:
            return "email rejected";
        case errc::EmailRequiresValidation:
            return "email requires validation";
        case errc::FacebookAccessTokenRequired:
            return "facebook access token required";
        case errc::FacebookEmailRegisteredWithOtherAccount:
            return "facebook registered with other account";
        case errc::FacebookAuthenticationFailure:
            return "facebook authentication failure";
        case errc::FileAlreadyExists:
            return "file already exists";
        case errc::FileAlreadyReported:
            return "file already reported";
        case errc::FileRemoved:
            return "file removed";
        case errc::FileUpdateDuplicate:
            return "file update duplicate";
        case errc::FilenameInvalid:
            return "filename invalid";
        case errc::FiletypeInvalid:
            return "filetype invalid";
        case errc::FolderAlreadyExists:
            return "folder already exists";
        case errc::FolderDepthLimitExceeded:
            return "folder depth limit exceeded";
        case errc::FolderPrivacyRestriction:
            return "folder privacy restriction";
        case errc::FolderkeyInvalid:
            return "folderkey invalid";
        case errc::FolderkeyMissing:
            return "folderkey missing";
        case errc::FoldernameMissing:
            return "foldername missing";
        case errc::GoogleAuthenticationFailure:
            return "google authentication failure";
        case errc::GroupAlreadyAdded:
            return "group already added";
        case errc::GroupInvalid:
            return "group invalid";
        case errc::IllegalEmail:
            return "illegal email";
        case errc::IllegalMove:
            return "illegal move";
        case errc::IllegalRename:
            return "illegal rename";
        case errc::IllegalUnshare:
            return "illegal unshare";
        case errc::InstallIdIncidentIdInvalidMatch:
            return "install id incident id invalid match";
        case errc::InstallIdInvalid:
            return "install id invalid";
        case errc::InsufficientCredits:
            return "insufficient credits";
        case errc::KeyInvalid:
            return "key invalid";
        case errc::KeyMissing:
            return "key missing";
        case errc::LoginRequired:
            return "login required";
        case errc::MessageSendFailure:
            return "message send failure";
        case errc::NoAvatarImage:
            return "no avatar image";
        case errc::NoInvoiceForUser:
            return "no invoice for user";
        case errc::NonPremiumLimitReached:
            return "non premium limit reached";
        case errc::ParametersInvalid:
            return "parameters invalid";
        case errc::ParametersMissing:
            return "parameters missing";
        case errc::PasswordMisformatted:
            return "password misformatted";
        case errc::PlanBandwidthInsufficient:
            return "plan bandwidth insufficient";
        case errc::PlanBusinessChangeInvalid:
            return "plan business change invalid";
        case errc::PlanChangeIllegal:
            return "plan change illegal";
        case errc::PlanClassChangeInvalid:
            return "plan class change invalid";
        case errc::PlanExpirationLimitation:
            return "plan expiration limitation";
        case errc::PlanStorageInsufficient:
            return "plan storage insufficient";
        case errc::PremiumRequired:
            return "premium required";
        case errc::ProductChangeIllegal:
            return "product change illegal";
        case errc::ProductIdInvalid:
            return "product id invalid";
        case errc::QuickkeyInvalid:
            return "quickkey invalid";
        case errc::QuickkeyMissing:
            return "quickkey missing";
        case errc::RequiresBandwidthConfirmtion:
            return "requires bandwidth confirmtion";
        case errc::ResellerTosAcceptanceRequired:
            return "reseller tos acceptance required";
        case errc::ResourceAlreadyFollowed:
            return "resource already followed";
        case errc::ResourceAlreadyOwned:
            return "resource already owned";
        case errc::ResourceGrantPermissionFailure:
            return "resource grant permission failure";
        case errc::ResourcePermissionFailure:
            return "resource permission failure";
        case errc::ServiceUnrecognized:
            return "service unrecognized";
        case errc::SessionTokenInvalid:
            return "session token invalid";
        case errc::SessionTokenMissing:
            return "session token missing";
        case errc::ShareResourceLimit:
            return "share resource limit";
        case errc::SignatureInvalid:
            return "signature invalid";
        case errc::SoftwareTokenInvalid:
            return "software token invalid";
        case errc::StorageLimitExceeded:
            return "storage limit exceeded";
        case errc::SubdomainInvalid:
            return "subdomain invalid";
        case errc::TosAcceptanceRequired:
            return "tos acceptance required";
        case errc::TosTokenInvalid:
            return "tos token invalid";
        case errc::TwitterAccessTokenRequired:
            return "twitter access token required";
        case errc::UnableToShareFromAnon:
            return "unable to share from anon";
        case errc::UnableToShareWithSelf:
            return "unable to share with self";
        case errc::UploadFailed:
            return "upload failed";
        case errc::UploadKeyInvalid:
            return "upload key invalid";
        case errc::UrlInvalid:
            return "url invalid";
        case errc::UserInvalid:
            return "user invalid";
        case errc::AccountTemporarilyLocked:
            return "account temporarily locked";
        case errc::FileTooBig:
            return "file size exceeds maximum limit";
        default:
        {
            assert(!"Unimplemented condition");
            std::stringstream ss;
            ss << "Unknown API category: " << ev;
            return ss.str();
        }
    }
}

bool ApiConditionImpl::equivalent(
        const std::error_code& ec,
        int condition_code
    ) const NOEXCEPT
{
    using mf::api::errc;

    switch (static_cast<errc>(condition_code))
    {
        default:
            return false;
    }
}


}  // namespace

namespace mf {
namespace api {

std::error_condition make_error_condition(errc e)
{
    return std::error_condition(
            static_cast<int>(e),
            generic_api_category()
            );
}

const std::error_category& generic_api_category()
{
    static ApiConditionImpl instance;
    return instance;
}

}  // namespace api
}  // namespace mf
