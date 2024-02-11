#include "network/server.h"

#include <fmt/printf.h>

#include "network/io_service_pool.h"

namespace network {

Server::Server(asio::io_context& io_context, unsigned short port_num)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port_num)) {
    fmt::println("[{}]: Server start success on port {}", __func__, port_num);
    StartAccept();
}

void Server::DeleteSession(const std::string& uuid) { sessions_.erase(uuid); }

void Server::StartAccept() {
    auto session = std::make_shared<Session>(IOServicePool::Instance().GetIOService(), this);
    acceptor_.async_accept(session->Socket(), [this, session](const boost::system::error_code& error_code) {
        HandleAccept(session, error_code);
        StartAccept();
    });
}

void Server::HandleAccept(const std::shared_ptr<Session>& session, const boost::system::error_code& error_code) {
    if (!error_code) {
        session->Start();
        sessions_.emplace(session->GetUuid(), session);
    } else {
        fmt::println("[{}]: Error code = {}. Message: {}", __func__, error_code.value(), error_code.message());
    }
}

}  // namespace network
