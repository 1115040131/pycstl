module chat.common.error_code;

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
