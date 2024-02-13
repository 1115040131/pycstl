#include "network/io_service_pool_server.h"

namespace network {

void IOServicePoolServer::StartAccept() {
    auto session = std::make_shared<Session>(IOServicePool::Instance().GetIOService(), this);
    acceptor_.async_accept(session->Socket(), [this, session](const boost::system::error_code& error_code) {
        HandleAccept(session, error_code);
        StartAccept();
    });
}

}  // namespace network
