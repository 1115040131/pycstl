#pragma once

#include <memory>
#include <unordered_map>

#include "gate_server/define.h"

namespace pyc {
namespace chat {

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
    friend class LogicSystem;

public:
    HttpConnection(asio::io_context& io_context);

    void Start();

    tcp::socket& GetSocket() { return socket_; }

    const std::unordered_map<std::string, std::string>& GetParams() const { return get_params_; }

private:
    void HandleRequest();

    void WriteResponse();

    void CheckDeadline();

    void ParseParams();

private:
    tcp::socket socket_;
    beast::flat_buffer buffer_{8192};
    http::request<http::dynamic_body> request_;
    http::response<http::dynamic_body> response_;
    asio::steady_timer dealine_{socket_.get_executor(), std::chrono::seconds(60)};

    std::string get_url_;
    std::unordered_map<std::string, std::string> get_params_;
};

}  // namespace chat
}  // namespace pyc
