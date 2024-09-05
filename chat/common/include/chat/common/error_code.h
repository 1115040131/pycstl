#pragma once

enum class ErrorCode {
    kSuccess,
    kJsonError,     // json 解析失败
    kRpcFailed,     // rpc 调用失败
    kNetworkError,  // 网络错误
};