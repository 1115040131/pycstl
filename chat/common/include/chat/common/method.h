#pragma once

enum class ReqId {
    kGetVerifyCode = 1001,  // 获取验证码
    kRegUser = 1002,        // 注册用户
    kResetPassword = 1003,   // 重置密码
    kLogin = 1004,          // 登录
    kChatLogin = 1005,      // 聊天服务器登录
    kChatLoginRes = 1006,   // 聊天服务器登录回复
};

