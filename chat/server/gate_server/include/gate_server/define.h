#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "logger/logger.h"

namespace pyc {
namespace chat {

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

enum class ErrorCode {
    kSuccess,
    kJsonError,     // json 解析失败
    kRpcFailed,     // rpc 调用失败
    kNetworkError,  // 网络错误
};

inline Logger g_logger("GateServer");

}  // namespace chat
}  // namespace pyc

#define PYC_LOG_DEBUG(...) ::pyc::chat::g_logger.debug(__VA_ARGS__)
#define PYC_LOG_INFO(...) ::pyc::chat::g_logger.info(__VA_ARGS__)
#define PYC_LOG_WARN(...) ::pyc::chat::g_logger.warn(__VA_ARGS__)
#define PYC_LOG_ERROR(...) ::pyc::chat::g_logger.error(__VA_ARGS__)
#define PYC_LOG_FATAL(...) ::pyc::chat::g_logger.fatal(__VA_ARGS__)
