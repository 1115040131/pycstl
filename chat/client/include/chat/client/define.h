#pragma once

#include "chat/common/error_code.h"

enum class ReqId {
    kGetVarifyCode = 1001,  // 获取验证码
    kRegUser = 1002,        // 注册用户
};

enum class Module {
    kRegisterMod = 0,
};
