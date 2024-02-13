#include "network/thread_pool_server/server.h"

#include "network/base/session.h"

namespace network {

void ThreadPoolServer::StartAccept() {
    auto session = std::make_shared<Session>(io_context_, this);
    acceptor_.async_accept(session->Socket(), [this, session](const boost::system::error_code& error_code) {
        HandleAccept(session, error_code);
        StartAccept();
    });
}

}  // namespace network
