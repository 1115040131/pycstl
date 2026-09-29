module;

#include <cstdint>

module chat.common.method;

#define TO_STRING_CASE(enum, code) \
    case enum ::code:              \
        return #code

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
