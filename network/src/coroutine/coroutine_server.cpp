#include "network/coroutine/coroutine_server.h"

#include "network/coroutine/coroutine_session.h"

namespace network {

void CoroutineServer::StartAccept() {
    auto session = std::make_shared<CoroutineSession>(IOServicePool::Instance().GetIOService(), this);
    acceptor_.async_accept(session->Socket(), [this, session](const boost::system::error_code& error_code) {
        HandleAccept(session, error_code);
        StartAccept();
    });
}

}  // namespace network
