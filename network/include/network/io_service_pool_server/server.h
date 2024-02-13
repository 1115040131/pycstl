#pragma once

#include "network/base/server.h"
#include "network/io_service_pool_server/pool.h"

namespace network {

class IOServicePoolServer : public Server {
public:
    using Server::Server;

    void StartAccept() override;
};

}  // namespace network
