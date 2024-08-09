#pragma once

#include <memory>

#include "gate_server/define.h"

namespace pyc {
namespace chat {

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
    friend class LogicSystem;

public:
    HttpConnection(tcp::socket socket);

    void Start();

private:
    void HandleRequest();

    void WriteResponse();

    void CheckDeadline();

private:
    tcp::socket socket_;
    beast::flat_buffer buffer_{8192};
    http::request<http::dynamic_body> request_;
    http::response<http::dynamic_body> response_;
    asio::steady_timer dealine_{socket_.get_executor(), std::chrono::seconds(60)};
};

}  // namespace chat
}  // namespace pyc
