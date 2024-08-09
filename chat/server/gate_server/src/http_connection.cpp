#include "gate_server/http_connection.h"

#include <iostream>

#include <fmt/ostream.h>

#include "gate_server/logic_system.h"

namespace pyc {
namespace chat {

HttpConnection::HttpConnection(tcp::socket socket) : socket_(std::move(socket)) {}

void HttpConnection::Start() {
    http::async_read(socket_, buffer_, request_,
                     [self = shared_from_this()](const beast::error_code& ec, std::size_t) {
                         try {
                             if (ec) {
                                 fmt::println(std::cerr, "{}", ec.what());
                                 return;
                             }

                             self->HandleRequest();
                             self->CheckDeadline();
                         } catch (const std::exception& e) {
                             fmt::print(std::cerr, "{}\n", e.what());
                         }
                     });
}

void HttpConnection::HandleRequest() {
    response_.version(request_.version());
    response_.keep_alive(false);
    if (request_.method() == http::verb::get) {
        bool success = LogicSystem::GetInstance().HandleGet(request_.target(), shared_from_this());
        if (!success) {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "Url not found\r\n";
            WriteResponse();
            return;
        }

        response_.result(http::status::ok);
        response_.set(http::field::server, "GateServer");
        WriteResponse();
        return;
    }
}

void HttpConnection::WriteResponse() {
    response_.content_length(response_.body().size());
    http::async_write(socket_, response_, [self = shared_from_this()](beast::error_code ec, std::size_t) {
        self->socket_.shutdown(tcp::socket::shutdown_send, ec);
        self->dealine_.cancel();
    });
}

void HttpConnection::CheckDeadline() {
    dealine_.async_wait([self = shared_from_this()](const beast::error_code& ec) {
        if (!ec) {
            self->socket_.close();
        }
    });
}

}  // namespace chat
}  // namespace pyc
