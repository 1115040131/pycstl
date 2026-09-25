export module network.coroutine_server;

export import network.base;

export namespace network {

class CoroutineServer : public Server {
public:
    using Server::Server;

    void StartAccept() override;
};

}  // namespace network
