export module network.io_service_pool_server;

export import network.base;

export namespace network {

class IOServicePoolServer : public Server {
public:
    using Server::Server;

    void StartAccept() override;
};

}  // namespace network
