module;

#include <memory>

#include <boost/asio.hpp>

module network.io_service_pool_server;

import network.io_service_pool;
import network.io_service_pool_session;

namespace network {

void IOServicePoolServer::StartAccept() {
    auto session = std::make_shared<IOServicePoolSession>(IOServicePool::GetInstance().GetIOService(), this);
    acceptor_.async_accept(session->Socket(), [this, session](const boost::system::error_code& error_code) {
        HandleAccept(session, error_code);
        StartAccept();
    });
}

}  // namespace network
