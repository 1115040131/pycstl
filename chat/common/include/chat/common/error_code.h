#pragma once

enum class ErrorCode {
    kSuccess = 0,

    kJsonError = 1001,     // json 解析失败
    kRpcFailed = 1002,     // rpc 调用失败
    kNetworkError = 1003,  // 网络错误

    // Gate Server
    kVerifyExpired = 2001,       // 验证码过期
    kVerifyCodeError = 2002,     // 验证码错误
    kUserExist = 2003,           // 用户已存在
    kPasswordError = 2004,       // 密码错误
    kEmailNotMatch = 2005,       // 邮箱不匹配
    kPasswordUpdateFail = 2006,  // 密码更新失败
    kPasswordInvalid = 2007,     // 密码不合法

    // Status Server
    kUidInvalid = 3001,    // uid 无效
    kTokenInvalid = 3002,  // token 无效
};

const char* ToString(ErrorCode err) noexcept;