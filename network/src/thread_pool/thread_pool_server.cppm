export module network.thread_pool_server;

export import network.base;

export namespace network {

class ThreadPoolServer : public Server {
public:
    using Server::Server;

    void StartAccept() override;
};

}  // namespace network
