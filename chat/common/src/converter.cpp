#include "chat/common/error_code.h"
#include "chat/common/method.h"

#define TO_STRING_CASE(enum, code) \
    case enum ::code:              \
        return #code

const char* ToString(ErrorCode err) noexcept {
    switch (err) {
        TO_STRING_CASE(ErrorCode, kSuccess);

        TO_STRING_CASE(ErrorCode, kJsonError);
        TO_STRING_CASE(ErrorCode, kRpcFailed);
        TO_STRING_CASE(ErrorCode, kNetworkError);

        TO_STRING_CASE(ErrorCode, kVerifyExpired);
        TO_STRING_CASE(ErrorCode, kVerifyCodeError);
        TO_STRING_CASE(ErrorCode, kUserExist);
        TO_STRING_CASE(ErrorCode, kPasswordError);
        TO_STRING_CASE(ErrorCode, kEmailNotMatch);
        TO_STRING_CASE(ErrorCode, kPasswordUpdateFail);
        TO_STRING_CASE(ErrorCode, kPasswordInvalid);

        TO_STRING_CASE(ErrorCode, kUidInvalid);
        TO_STRING_CASE(ErrorCode, kTokenInvalid);
        default:
            break;
    }
    return "Unknown";
}

const char* ToString(ReqId req_id) noexcept {
    switch (req_id) {
        TO_STRING_CASE(ReqId, kGetVerifyCode);
        TO_STRING_CASE(ReqId, kRegUser);
        TO_STRING_CASE(ReqId, kResetPassword);
        TO_STRING_CASE(ReqId, kLogin);
        TO_STRING_CASE(ReqId, kChatLogin);
        TO_STRING_CASE(ReqId, kChatLoginRes);
        TO_STRING_CASE(ReqId, kSearchUserReq);
        TO_STRING_CASE(ReqId, kSearchUserRes);
        TO_STRING_CASE(ReqId, kAddFriendReq);
        TO_STRING_CASE(ReqId, kAddFriendRes);
        TO_STRING_CASE(ReqId, kNotifyAddFriendReq);
        TO_STRING_CASE(ReqId, kAuthFriendReq);
        TO_STRING_CASE(ReqId, kAuthFriendRes);
        TO_STRING_CASE(ReqId, kNotifyAuthFriendReq);
        TO_STRING_CASE(ReqId, kTextChatMsgReq);
        TO_STRING_CASE(ReqId, kTextChatMsgRes);
        TO_STRING_CASE(ReqId, kNotifyTextChatMsgReq);

        default:
            break;
    }
    return "Unknown";
}

const char* ToUrl(ReqId req_id) noexcept {
    switch (req_id) {
        case ReqId::kGetVerifyCode:
            return "/get_verifycode";
        case ReqId::kRegUser:
            return "/user_register";
        case ReqId::kResetPassword:
            return "/reset_password";
        case ReqId::kLogin:
            return "/user_login";
        default:
            break;
    }
    return "Unknown";
}