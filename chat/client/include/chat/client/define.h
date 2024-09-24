#pragma once

#include "chat/common/error_code.h"

enum class ReqId {
    kGetVerifyCode = 1001,  // 获取验证码
    kRegUser = 1002,        // 注册用户
    kRestPassword = 1003,   // 重置密码
    kLogin = 1004,          // 登录
    kChatLogin = 1005,      // 聊天服务器登录
    kChatLoginRes = 1006,   // 聊天服务器登录回复
};

enum class Module {
    kRegisterMod = 0,
    kResetMod,
};

enum class TipErr {
    kSuccess = 0,
    kEmailErr,
    kPasswordErr,
    kConfirmErr,
    kPasswordConfirm,
    kVerifyErr,
    kUserErr,
};