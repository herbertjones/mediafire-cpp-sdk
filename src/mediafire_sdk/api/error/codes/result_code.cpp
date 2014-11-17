/**
 * @file result_code.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "result_code.hpp"

#include <string>
#include <sstream>

#include "mediafire_sdk/api/error/conditions/generic.hpp"
#include "mediafire_sdk/utils/noexcept.hpp"

namespace {

class CategoryImpl
    : public std::error_category
{
public:
  virtual const char* name() const NOEXCEPT;
  virtual std::string message(int ev) const;
  virtual std::error_condition default_error_condition(int ev) const NOEXCEPT;
};

const char* CategoryImpl::name() const NOEXCEPT
{
    return "mediafire api error response";
}

std::string CategoryImpl::message(int ev) const
{
    using mf::api::result_code;

    switch (static_cast<result_code>(ev))
    {
        case result_code::SessionTokenInvalid:
            return "session token invalid";
        case result_code::CredentialsInvalid:
            return "credentials invalid";
        case result_code::SignatureInvalid:
            return "signature invalid";
        case result_code::AsyncOperationInProgress:
            return "async operation in progress";
        default:
        {
            std::ostringstream ss;
            ss << "API result code: " << ev;
            return ss.str();
        }
    }
}

std::error_condition CategoryImpl::default_error_condition(
        int ev
    ) const NOEXCEPT
{
    namespace api = mf::api;

    switch (ev)
    {
        case 100: return api::errc::ApiInternalServerError;
        case 102: return api::errc::KeyMissing;
        case 103: return api::errc::KeyInvalid;
        case 104: return api::errc::SessionTokenMissing;
        case 105: return api::errc::SessionTokenInvalid;
        case 106: return api::errc::IllegalRename;
        case 107: return api::errc::CredentialsInvalid;
        case 108: return api::errc::UserInvalid;
        case 109: return api::errc::ApplicationInvalid;
        case 110: return api::errc::QuickkeyInvalid;
        case 111: return api::errc::QuickkeyMissing;
        case 112: return api::errc::FolderkeyInvalid;
        case 113: return api::errc::FolderkeyMissing;
        case 114: return api::errc::AccessDenied;
        case 115: return api::errc::IllegalMove;
        case 116: return api::errc::DateInvalid;
        case 117: return api::errc::FoldernameMissing;
        case 118: return api::errc::FilenameInvalid;
        case 119: return api::errc::IllegalEmail;
        case 120: return api::errc::EmailAlreadyInUse;
        case 121: return api::errc::EmailRejected;
        case 122: return api::errc::EmailMisformatted;
        case 123: return api::errc::PasswordMisformatted;
        case 124: return api::errc::ApiVersionMissing;
        case 125: return api::errc::ApiVersionRemoved;
        case 126: return api::errc::ApiVersionDeprecated;
        case 127: return api::errc::SignatureInvalid;
        case 128: return api::errc::ParametersMissing;
        case 129: return api::errc::ParametersInvalid;
        case 130: return api::errc::NonPremiumLimitReached;
        case 131: return api::errc::UnableToShareWithSelf;
        case 132: return api::errc::IllegalUnshare;
        case 133: return api::errc::UnableToShareFromAnon;
        case 134: return api::errc::DmcaInvalidUser;
        case 135: return api::errc::DmcaInvalidRange;
        case 136: return api::errc::DmcaBan;
        case 137: return api::errc::ContactAlreadyAdded;
        case 138: return api::errc::ContactInvalid;
        case 139: return api::errc::GroupAlreadyAdded;
        case 140: return api::errc::GroupInvalid;
        case 141: return api::errc::DeviceInvalid;
        case 142: return api::errc::FiletypeInvalid;
        case 143: return api::errc::FileAlreadyExists;
        case 144: return api::errc::FolderAlreadyExists;
        case 145: return api::errc::ApplicationIdDisabled;
        case 146: return api::errc::ApplicationIdSuspended;
        case 147: return api::errc::BulkOperationInvalid;
        case 148: return api::errc::BulkOperationIllegal;
        case 149: return api::errc::RequiresBandwidthConfirmtion;
        case 150: return api::errc::BulkFilesizeLimitExceeded;
        case 151: return api::errc::BulkFileCountZero;
        case 152: return api::errc::BulkPackageFailure;
        case 153: return api::errc::BulkFilesizeLimitConfirmationNeeded;
        case 154: return api::errc::BulkFileCountExceeded;
        case 155: return api::errc::BulkRequiresBandwidthConfirmtion;
        case 156: return api::errc::BulkInsufficientPremiumBandwidth;
        case 157: return api::errc::BulkInsufficientUnifiedBandwidth;
        case 158: return api::errc::FileAlreadyExists;
        case 159: return api::errc::FolderAlreadyExists;
        case 160: return api::errc::TosTokenInvalid;
        case 161: return api::errc::TosAcceptanceRequired;
        case 162: return api::errc::StorageLimitExceeded;
        case 163: return api::errc::ApiRequestLimitExceeded;
        case 164: return api::errc::FileAlreadyReported;
        case 165: return api::errc::FileRemoved;
        case 166: return api::errc::FolderPrivacyRestriction;
        case 167: return api::errc::FolderDepthLimitExceeded;
        case 168: return api::errc::ProductIdInvalid;
        case 169: return api::errc::UploadFailed;
        case 170: return api::errc::PlanClassChangeInvalid;
        case 171: return api::errc::PlanBusinessChangeInvalid;
        case 172: return api::errc::PlanExpirationLimitation;
        case 173: return api::errc::PremiumRequired;
        case 174: return api::errc::UrlInvalid;
        case 175: return api::errc::UploadKeyInvalid;
        case 176: return api::errc::PlanStorageInsufficient;
        case 177: return api::errc::DuplicateEntry;
        case 178: return api::errc::PlanChangeIllegal;
        case 179: return api::errc::ProductChangeIllegal;
        case 180: return api::errc::DowngradeIllegal;
        case 181: return api::errc::BusinessUpgradeFailure;
        case 182: return api::errc::InsufficientCredits;
        case 183: return api::errc::PlanBandwidthInsufficient;
        case 184: return api::errc::AccountAlreadyLinked;
        case 188: return api::errc::LoginRequired;
        case 189: return api::errc::ResellerTosAcceptanceRequired;
        case 190: return api::errc::BusinessSeatLimitation;
        case 191: return api::errc::BillingFailure;
        case 192: return api::errc::BillingFailure;
        case 193: return api::errc::BillingFailure;
        case 194: return api::errc::SubdomainInvalid;
        case 195: return api::errc::BillingFailure;
        case 196: return api::errc::CreditCardInvalid;
        case 197: return api::errc::BillingFailure;
        case 198: return api::errc::BillingFailure;
        case 199: return api::errc::BillingTransactionDuplicate;
        case 200: return api::errc::CreditCardInvalid;
        case 201: return api::errc::BillingFailure;
        case 202: return api::errc::BillingFailure;
        case 203: return api::errc::BillingFailure;
        case 204: return api::errc::BillingFailure;
        case 205: return api::errc::BillingFailure;
        case 206: return api::errc::InternalServerError;
        case 208: return api::errc::AsyncOperationInProgress;
        case 219: return api::errc::FacebookEmailRegisteredWithOtherAccount;
        case 220: return api::errc::FacebookAuthenticationFailure;
        case 224: return api::errc::NoInvoiceForUser;
        case 225: return api::errc::InternalServerError;
        case 226: return api::errc::InstallIdInvalid;
        case 227: return api::errc::InstallIdIncidentIdInvalidMatch;
        case 228: return api::errc::FacebookAccessTokenRequired;
        case 229: return api::errc::TwitterAccessTokenRequired;
        case 230: return api::errc::NoAvatarImage;
        case 231: return api::errc::SoftwareTokenInvalid;
        case 232: return api::errc::EmailRequiresValidation;
        case 233: return api::errc::GoogleAuthenticationFailure;
        case 234: return api::errc::MessageSendFailure;
        case 235: return api::errc::ResourceAlreadyOwned;
        case 236: return api::errc::ResourceAlreadyFollowed;
        case 237: return api::errc::ResourcePermissionFailure;
        case 238: return api::errc::FileUpdateDuplicate;
        case 239: return api::errc::ShareResourceLimit;
        case 240: return api::errc::ResourceGrantPermissionFailure;
        case 241: return api::errc::ServiceUnrecognized;
        case 243: return api::errc::AccountTemporarilyLocked;
        default: return std::error_condition(ev, *this);
    }
}

}  // namespace

namespace mf {
namespace api {

const std::error_category& result_category()
{
    static CategoryImpl instance;
    return instance;
}

std::error_code make_error_code(result_code e)
{
    return std::error_code(
            static_cast<int>(e),
            mf::api::result_category()
            );
}

std::error_condition make_error_condition(result_code e)
{
    return std::error_condition(
            static_cast<int>(e),
            mf::api::result_category()
            );
}

}  // End namespace api
}  // namespace mf
