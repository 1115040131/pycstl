#pragma once

enum class ErrorCode {
    kSuccess = 0,

    kJsonError = 1001,           // json 解析失败
    kRpcFailed = 1002,           // rpc 调用失败
    kVerifyExpired = 1003,       // 验证码过期
    kVerifyCodeError = 1004,     // 验证码错误
    kUserExist = 1005,           // 用户已存在
    kPasswordError = 1006,       // 密码错误
    kEmailNotMatch = 1007,       // 邮箱不匹配
    kPasswordUpdateFail = 1008,  // 密码更新失败
    kPasswordInvalid = 1009,     // 密码不合法

    kNetworkError = 1010,  // 网络错误
};