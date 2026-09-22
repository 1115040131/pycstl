export module network.io_service_pool_session;

export import network.base;

export namespace network {

class IOServicePoolSession : public Session {
public:
    using Session::Session;

    ~IOServicePoolSession() override;

    void Start() override;

protected:
    void AsyncRead() override;
    void AsyncWrite() override;
};

}  // namespace network
