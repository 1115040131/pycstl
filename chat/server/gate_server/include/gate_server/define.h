#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

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

}  // namespace chat
}  // namespace pyc