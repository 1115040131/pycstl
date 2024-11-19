#pragma once

#include <cstdint>

enum class ReqId : uint16_t {
    kInvalid = 0,  // 无效请求

    kGetVerifyCode = 1001,  // 获取验证码
    kRegUser = 1002,        // 注册用户
    kResetPassword = 1003,  // 重置密码
    kLogin = 1004,          // 登录

    kChatLogin = 1005,     // 聊天服务器登录
    kChatLoginRes = 1006,  // 聊天服务器登录回复

    kSearchUserReq = 1007,  // 搜索用户请求
    kSearchUserRes = 1008,  // 搜索用户回复

    kAddFriendReq = 1009,        // 添加好友请求
    kAddFriendRes = 1010,        // 添加好友回复
    kNotifyAddFriendReq = 1011,  // 通知添加好友请求

    kAuthFriendReq = 1013,        // 审核好友请求
    kAuthFriendRes = 1014,        // 审核好友回复
    kNotifyAuthFriendReq = 1015,  // 通知审核好友请求

    kTextChatMsgReq = 1017,        // 文本聊天消息请求
    kTextChatMsgRes = 1018,        // 文本聊天消息回复
    kNotifyTextChatMsgReq = 1019,  // 通知文本聊天消息请求
};

const char* ToString(ReqId req_id) noexcept;

const char* ToUrl(ReqId req_id) noexcept;