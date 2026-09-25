module;

#include <memory>

#include <boost/asio.hpp>

module network.thread_pool_server;

import network.thread_pool_session;

namespace network {

void ThreadPoolServer::StartAccept() {
    auto session = std::make_shared<ThreadPoolSession>(io_context_, this);
    acceptor_.async_accept(session->Socket(), [this, session](const boost::system::error_code& error_code) {
        HandleAccept(session, error_code);
        StartAccept();
    });
}

}  // namespace network
