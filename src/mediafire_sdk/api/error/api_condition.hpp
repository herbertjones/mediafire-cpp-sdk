/**
 * @file api_condition.hpp
 * @author Herbert Jones
 * @brief API error conditions
 * @copyright Copyright 2014 Mediafire
 */
#pragma once

#include <string>
#include <system_error>

#include "mediafire_sdk/utils/noexcept.hpp"

namespace mf {
namespace api {

/**
 * api error conditions
 *
 * You may compare these to API errors and they may be used as generic errors.
 */
enum class errc
{
    ContentInvalidFormat,
    ContentInvalidData,
    BadRequest,
    Forbidden,
    NotFound,
    InternalServerError,
    ApiInternalServerError,
    UnknownApiError,
    SessionTokenUnavailableTimeout,
    ConnectionUnavailableTimeout,

    // Api error code errors

    AccessDenied,
    AccountAlreadyLinked,
    ApiRequestLimitExceeded,
    ApiVersionDeprecated,
    ApiVersionMissing,
    ApiVersionRemoved,
    ApplicationIdDisabled,
    ApplicationIdSuspended,
    ApplicationInvalid,
    AsyncOperationInProgress,
    BillingFailure,
    BillingTransactionDuplicate,
    BulkFileCountExceeded,
    BulkFileCountZero,
    BulkFilesizeLimitConfirmationNeeded,
    BulkFilesizeLimitExceeded,
    BulkInsufficientPremiumBandwidth,
    BulkInsufficientUnifiedBandwidth,
    BulkOperationIllegal,
    BulkOperationInvalid,
    BulkPackageFailure,
    BulkRequiresBandwidthConfirmtion,
    BusinessSeatLimitation,
    BusinessUpgradeFailure,
    ContactAlreadyAdded,
    ContactInvalid,
    CredentialsInvalid,
    CreditCardInvalid,
    DateInvalid,
    DeviceInvalid,
    DmcaBan,
    DmcaInvalidRange,
    DmcaInvalidUser,
    DowngradeIllegal,
    DuplicateEntry,
    EmailAlreadyInUse,
    EmailMisformatted,
    EmailRejected,
    EmailRequiresValidation,
    FacebookAccessTokenRequired,
    FacebookEmailRegisteredWithOtherAccount,
    FacebookAuthenticationFailure,
    FileAlreadyExists,
    FileAlreadyReported,
    FileRemoved,
    FileUpdateDuplicate,
    FilenameInvalid,
    FiletypeInvalid,
    FolderAlreadyExists,
    FolderDepthLimitExceeded,
    FolderPrivacyRestriction,
    FolderkeyInvalid,
    FolderkeyMissing,
    FoldernameMissing,
    GoogleAuthenticationFailure,
    GroupAlreadyAdded,
    GroupInvalid,
    IllegalEmail,
    IllegalMove,
    IllegalRename,
    IllegalUnshare,
    InstallIdIncidentIdInvalidMatch,
    InstallIdInvalid,
    InsufficientCredits,
    KeyInvalid,
    KeyMissing,
    LoginRequired,
    MessageSendFailure,
    NoAvatarImage,
    NoInvoiceForUser,
    NonPremiumLimitReached,
    ParametersInvalid,
    ParametersMissing,
    PasswordMisformatted,
    PlanBandwidthInsufficient,
    PlanBusinessChangeInvalid,
    PlanChangeIllegal,
    PlanClassChangeInvalid,
    PlanExpirationLimitation,
    PlanStorageInsufficient,
    PremiumRequired,
    ProductChangeIllegal,
    ProductIdInvalid,
    QuickkeyInvalid,
    QuickkeyMissing,
    RequiresBandwidthConfirmtion,
    ResellerTosAcceptanceRequired,
    ResourceAlreadyFollowed,
    ResourceAlreadyOwned,
    ResourceGrantPermissionFailure,
    ResourcePermissionFailure,
    ServiceUnrecognized,
    SessionTokenInvalid,
    SessionTokenMissing,
    ShareResourceLimit,
    SignatureInvalid,
    SoftwareTokenInvalid,
    StorageLimitExceeded,
    SubdomainInvalid,
    TosAcceptanceRequired,
    TosTokenInvalid,
    TwitterAccessTokenRequired,
    UnableToShareFromAnon,
    UnableToShareWithSelf,
    UploadFailed,
    UploadKeyInvalid,
    UrlInvalid,
    UserInvalid,
};

/**
 * @brief Create an error condition for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error condition
 */
std::error_condition make_error_condition(errc e);

/**
 * @brief Create an error code for std::error_code usage.
 *
 * @param[in] e Error code
 *
 * @return Error code
 */
std::error_code make_error_code(errc e);

/**
 * @brief Create/get the instance of the error category.
 *
 * @return The std::error_category beloging to our error codes.
 */
const std::error_category& generic_api_category();

}  // End namespace api
}  // namespace mf

namespace std
{
    template <>
    struct is_error_condition_enum<mf::api::errc>
        : public true_type {};
}  // End namespace std

