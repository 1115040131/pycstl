#include "network/websocket/websocket_server.h"

#include <fmt/core.h>

#include "network/websocket/connection.h"

namespace network {

WebsocketServer::WebsocketServer(asio::io_context& io_context, unsigned short port)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    fmt::println("[{}] Server started on port {}", __func__, port);
}

void WebsocketServer::StartAccept() {
    auto connection = std::make_shared<Connection>(io_context_);
    acceptor_.async_accept(connection->Socket(), [this, connection](const boost::system::error_code& error_code) {
        if (!error_code) {
            connection->StartAccept();
        } else {
            fmt::println("[async_accept]: Error code: {}. Message: {}", error_code.value(), error_code.message());
        }
        StartAccept();
    });
}

}  // namespace network
