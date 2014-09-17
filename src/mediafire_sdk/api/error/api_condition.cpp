/**
 * @file api_condition.cpp
 * @author Herbert Jones
 * @copyright Copyright 2014 Mediafire
 */
#include "api_condition.hpp"

#include <cassert>
#include <sstream>
#include <string>

#include "api_category.hpp"

namespace mf {
namespace api {

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

std::error_condition make_error_condition(errc e)
{
    return std::error_condition(
            static_cast<int>(e),
            generic_api_category()
            );
}

std::error_code make_error_code(errc e)
{
    return std::error_code(
            static_cast<int>(e),
            generic_api_category()
            );
}

const std::error_category& generic_api_category()
{
    static ApiConditionImpl instance;
    return instance;
}

const char* ApiConditionImpl::name() const NOEXCEPT
{
    return "api";
}

std::string ApiConditionImpl::message(int ev) const
{
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
    switch (static_cast<errc>(condition_code))
    {
        case errc::ContentInvalidFormat:
            return false;
        case errc::ContentInvalidData:
            return false;
        case errc::BadRequest:
            return false;
        case errc::Forbidden:
            return false;
        case errc::NotFound:
            return false;
        case errc::InternalServerError:
            return ec == std::error_code(206, api_category())
                || ec == std::error_code(225, api_category());
        case errc::ApiInternalServerError:
            return ec == std::error_code(100, api_category());
        case errc::UnknownApiError:
            return false;
        case errc::SessionTokenUnavailableTimeout:
            return false;
        case errc::ConnectionUnavailableTimeout:
            return false;
        case errc::AccessDenied:
            return ec == std::error_code(114, api_category());
        case errc::AccountAlreadyLinked:
            return ec == std::error_code(184, api_category());
        case errc::ApiRequestLimitExceeded:
            return ec == std::error_code(163, api_category());
        case errc::ApiVersionDeprecated:
            return ec == std::error_code(126, api_category());
        case errc::ApiVersionMissing:
            return ec == std::error_code(124, api_category());
        case errc::ApiVersionRemoved:
            return ec == std::error_code(125, api_category());
        case errc::ApplicationIdDisabled:
            return ec == std::error_code(145, api_category());
        case errc::ApplicationIdSuspended:
            return ec == std::error_code(146, api_category());
        case errc::ApplicationInvalid:
            return ec == std::error_code(109, api_category());
        case errc::AsyncOperationInProgress:
            return ec == std::error_code(208, api_category());
        case errc::BillingFailure:
            return ec == std::error_code(191, api_category())
                || ec == std::error_code(192, api_category())
                || ec == std::error_code(193, api_category())
                || ec == std::error_code(195, api_category())
                || ec == std::error_code(197, api_category())
                || ec == std::error_code(198, api_category())
                || ec == std::error_code(201, api_category())
                || ec == std::error_code(202, api_category())
                || ec == std::error_code(203, api_category())
                || ec == std::error_code(204, api_category())
                || ec == std::error_code(205, api_category());
            return false;
        case errc::BillingTransactionDuplicate:
            return ec == std::error_code(199, api_category());
        case errc::BulkFileCountExceeded:
            return ec == std::error_code(154, api_category());
        case errc::BulkFileCountZero:
            return ec == std::error_code(151, api_category());
        case errc::BulkFilesizeLimitConfirmationNeeded:
            return ec == std::error_code(153, api_category());
        case errc::BulkFilesizeLimitExceeded:
            return ec == std::error_code(150, api_category());
        case errc::BulkInsufficientPremiumBandwidth:
            return ec == std::error_code(156, api_category());
        case errc::BulkInsufficientUnifiedBandwidth:
            return ec == std::error_code(157, api_category());
        case errc::BulkOperationIllegal:
            return ec == std::error_code(148, api_category());
        case errc::BulkOperationInvalid:
            return ec == std::error_code(147, api_category());
        case errc::BulkPackageFailure:
            return ec == std::error_code(152, api_category());
        case errc::BulkRequiresBandwidthConfirmtion:
            return ec == std::error_code(155, api_category());
        case errc::BusinessSeatLimitation:
            return ec == std::error_code(190, api_category());
        case errc::BusinessUpgradeFailure:
            return ec == std::error_code(181, api_category());
        case errc::ContactAlreadyAdded:
            return ec == std::error_code(137, api_category());
        case errc::ContactInvalid:
            return ec == std::error_code(138, api_category());
        case errc::CredentialsInvalid:
            return ec == std::error_code(107, api_category());
        case errc::CreditCardInvalid:
            return ec == std::error_code(200, api_category())
                || ec == std::error_code(196, api_category());
        case errc::DateInvalid:
            return ec == std::error_code(116, api_category());
        case errc::DeviceInvalid:
            return ec == std::error_code(141, api_category());
        case errc::DmcaBan:
            return ec == std::error_code(136, api_category());
        case errc::DmcaInvalidRange:
            return ec == std::error_code(135, api_category());
        case errc::DmcaInvalidUser:
            return ec == std::error_code(134, api_category());
        case errc::DowngradeIllegal:
            return ec == std::error_code(180, api_category());
        case errc::DuplicateEntry:
            return ec == std::error_code(177, api_category());
        case errc::EmailAlreadyInUse:
            return ec == std::error_code(120, api_category());
        case errc::EmailMisformatted:
            return ec == std::error_code(122, api_category());
        case errc::EmailRejected:
            return ec == std::error_code(121, api_category());
        case errc::EmailRequiresValidation:
            return ec == std::error_code(232, api_category());
        case errc::FacebookAccessTokenRequired:
            return ec == std::error_code(228, api_category());
        case errc::FacebookEmailRegisteredWithOtherAccount:
            return ec == std::error_code(219, api_category());
        case errc::FacebookAuthenticationFailure:
            return ec == std::error_code(220, api_category());
        case errc::FileAlreadyExists:
            return ec == std::error_code(158, api_category())
                || ec == std::error_code(143, api_category());
        case errc::FileAlreadyReported:
            return ec == std::error_code(164, api_category());
        case errc::FileRemoved:
            return ec == std::error_code(165, api_category());
        case errc::FileUpdateDuplicate:
            return ec == std::error_code(238, api_category());
        case errc::FilenameInvalid:
            return ec == std::error_code(118, api_category());
        case errc::FiletypeInvalid:
            return ec == std::error_code(142, api_category());
        case errc::FolderAlreadyExists:
            return ec == std::error_code(159, api_category())
                || ec == std::error_code(144, api_category());
        case errc::FolderDepthLimitExceeded:
            return ec == std::error_code(167, api_category());
        case errc::FolderPrivacyRestriction:
            return ec == std::error_code(166, api_category());
        case errc::FolderkeyInvalid:
            return ec == std::error_code(112, api_category());
        case errc::FolderkeyMissing:
            return ec == std::error_code(113, api_category());
        case errc::FoldernameMissing:
            return ec == std::error_code(117, api_category());
        case errc::GoogleAuthenticationFailure:
            return ec == std::error_code(233, api_category());
        case errc::GroupAlreadyAdded:
            return ec == std::error_code(139, api_category());
        case errc::GroupInvalid:
            return ec == std::error_code(140, api_category());
        case errc::IllegalEmail:
            return ec == std::error_code(119, api_category());
        case errc::IllegalMove:
            return ec == std::error_code(115, api_category());
        case errc::IllegalRename:
            return ec == std::error_code(106, api_category());
        case errc::IllegalUnshare:
            return ec == std::error_code(132, api_category());
        case errc::InstallIdIncidentIdInvalidMatch:
            return ec == std::error_code(227, api_category());
        case errc::InstallIdInvalid:
            return ec == std::error_code(226, api_category());
        case errc::InsufficientCredits:
            return ec == std::error_code(182, api_category());
        case errc::KeyInvalid:
            return ec == std::error_code(103, api_category());
        case errc::KeyMissing:
            return ec == std::error_code(102, api_category());
        case errc::LoginRequired:
            return ec == std::error_code(188, api_category());
        case errc::MessageSendFailure:
            return ec == std::error_code(234, api_category());
        case errc::NoAvatarImage:
            return ec == std::error_code(230, api_category());
        case errc::NoInvoiceForUser:
            return ec == std::error_code(224, api_category());
        case errc::NonPremiumLimitReached:
            return ec == std::error_code(130, api_category());
        case errc::ParametersInvalid:
            return ec == std::error_code(129, api_category());
        case errc::ParametersMissing:
            return ec == std::error_code(128, api_category());
        case errc::PasswordMisformatted:
            return ec == std::error_code(123, api_category());
        case errc::PlanBandwidthInsufficient:
            return ec == std::error_code(183, api_category());
        case errc::PlanBusinessChangeInvalid:
            return ec == std::error_code(171, api_category());
        case errc::PlanChangeIllegal:
            return ec == std::error_code(178, api_category());
        case errc::PlanClassChangeInvalid:
            return ec == std::error_code(170, api_category());
        case errc::PlanExpirationLimitation:
            return ec == std::error_code(172, api_category());
        case errc::PlanStorageInsufficient:
            return ec == std::error_code(176, api_category());
        case errc::PremiumRequired:
            return ec == std::error_code(173, api_category());
        case errc::ProductChangeIllegal:
            return ec == std::error_code(179, api_category());
        case errc::ProductIdInvalid:
            return ec == std::error_code(168, api_category());
        case errc::QuickkeyInvalid:
            return ec == std::error_code(110, api_category());
        case errc::QuickkeyMissing:
            return ec == std::error_code(111, api_category());
        case errc::RequiresBandwidthConfirmtion:
            return ec == std::error_code(149, api_category());
        case errc::ResellerTosAcceptanceRequired:
            return ec == std::error_code(189, api_category());
        case errc::ResourceAlreadyFollowed:
            return ec == std::error_code(236, api_category());
        case errc::ResourceAlreadyOwned:
            return ec == std::error_code(235, api_category());
        case errc::ResourceGrantPermissionFailure:
            return ec == std::error_code(240, api_category());
        case errc::ResourcePermissionFailure:
            return ec == std::error_code(237, api_category());
        case errc::ServiceUnrecognized:
            return ec == std::error_code(241, api_category());
        case errc::SessionTokenInvalid:
            return ec == std::error_code(105, api_category());
        case errc::SessionTokenMissing:
            return ec == std::error_code(104, api_category());
        case errc::ShareResourceLimit:
            return ec == std::error_code(239, api_category());
        case errc::SignatureInvalid:
            return ec == std::error_code(127, api_category());
        case errc::SoftwareTokenInvalid:
            return ec == std::error_code(231, api_category());
        case errc::StorageLimitExceeded:
            return ec == std::error_code(162, api_category());
        case errc::SubdomainInvalid:
            return ec == std::error_code(194, api_category());
        case errc::TosAcceptanceRequired:
            return ec == std::error_code(161, api_category());
        case errc::TosTokenInvalid:
            return ec == std::error_code(160, api_category());
        case errc::TwitterAccessTokenRequired:
            return ec == std::error_code(229, api_category());
        case errc::UnableToShareFromAnon:
            return ec == std::error_code(133, api_category());
        case errc::UnableToShareWithSelf:
            return ec == std::error_code(131, api_category());
        case errc::UploadFailed:
            return ec == std::error_code(169, api_category());
        case errc::UploadKeyInvalid:
            return ec == std::error_code(175, api_category());
        case errc::UrlInvalid:
            return ec == std::error_code(174, api_category());
        case errc::UserInvalid:
            return ec == std::error_code(108, api_category());
        default:
            {
                assert(!"Unimplemented condition equivalence");
                return false;
            }
        // [207,223] skipped in wiki
        // [185,187] skipped in wiki
    }
}

}  // namespace api
}  // namespace mf
