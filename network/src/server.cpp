#include "network/server.h"

#include <fmt/printf.h>

namespace network {

Server::Server(asio::io_context& io_context, unsigned short port_num)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port_num)) {
    fmt::println("[{}]: Server start success on port {}", __func__, port_num);
}

void Server::DeleteSession(const std::string& uuid) { sessions_.erase(uuid); }

void Server::HandleAccept(const std::shared_ptr<Session>& session, const boost::system::error_code& error_code) {
    if (!error_code) {
        session->Start();
        sessions_.emplace(session->GetUuid(), session);
    } else {
        fmt::println("[{}]: Error code = {}. Message: {}", __func__, error_code.value(), error_code.message());
    }
}

}  // namespace network
