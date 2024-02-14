#pragma once

#include "network/base/session.h"

namespace network {

/// @brief 协程读取, 异步发送
class CoroutineSession : public Session {
public:
    CoroutineSession(asio::io_context& io_context, Server* server)
        : Session(io_context, server), io_context_(io_context) {}

    ~CoroutineSession() override;

    void Start() override;

protected:
    asio::awaitable<void> CoawaitRead();

    void AsyncRead() override {}
    void AsyncWrite() override;

private:
    asio::io_context& io_context_;
};

}  // namespace network
