#include "network/io_service_pool_server/server.h"

#include "network/io_service_pool_server/session.h"

namespace network {

void IOServicePoolServer::StartAccept() {
    auto session = std::make_shared<IOServicePoolSession>(IOServicePool::Instance().GetIOService(), this);
    acceptor_.async_accept(session->Socket(), [this, session](const boost::system::error_code& error_code) {
        HandleAccept(session, error_code);
        StartAccept();
    });
}

}  // namespace network
