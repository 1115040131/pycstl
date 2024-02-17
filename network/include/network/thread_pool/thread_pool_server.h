#pragma once

#include "network/base/server.h"
#include "network/thread_pool/thread_pool.h"

namespace network {

class ThreadPoolServer : public Server {
public:
    using Server::Server;

    void StartAccept() override;
};

}  // namespace network
