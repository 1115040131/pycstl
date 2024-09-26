#pragma once

#include "chat/common/method.h"

inline constexpr const char* ToUrl(ReqId req_id) {
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
    return "unknown";
}