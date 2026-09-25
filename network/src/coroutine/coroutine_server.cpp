module;

#include <memory>

#include <boost/asio.hpp>

module network.coroutine_server;

import network.coroutine_session;
import network.io_service_pool;

namespace network {

void CoroutineServer::StartAccept() {
    auto session = std::make_shared<CoroutineSession>(IOServicePool::GetInstance().GetIOService(), this);
    acceptor_.async_accept(session->Socket(), [this, session](const boost::system::error_code& error_code) {
        HandleAccept(session, error_code);
        StartAccept();
    });
}

}  // namespace network
