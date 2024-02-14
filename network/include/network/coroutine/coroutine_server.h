#pragma once

#include "network/base/server.h"
#include "network/io_service_pool/io_service_pool.h"

namespace network {

class CoroutineServer : public Server {
public:
    using Server::Server;

    void StartAccept() override;
};

}  // namespace network