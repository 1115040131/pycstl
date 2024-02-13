#pragma once

#include "network/io_service_pool.h"
#include "network/server.h"

namespace network {

class IOServicePoolServer : public Server {
public:
    using Server::Server;

    void StartAccept() override;
};

}  // namespace network
