#pragma once

enum class ReqId {
    kGetVarifyCode = 1001,  // 获取验证码
    kRegUser = 1002,        // 注册用户
};

enum class Module {
    kRegisterMod = 0,
};

enum class ErrorCode {
    kSuccess,
    kJsonError,     // json 解析失败
    kNetworkError,  // 网络错误
};