#pragma once

#include "network/base/session.h"

namespace network {

class ThreadPoolSession : public Session {
public:
    ThreadPoolSession(asio::io_context& io_context, Server* server);

    ~ThreadPoolSession() override;

    void Start() override;

protected:
    void AsyncRead() override;
    void AsyncWrite() override;

private:
    // 将 io_context 与 strand 关联, 当 socket 就绪后并不是由多个线程调用每个 socket 注册的回调函数,
    // 而是将回调函数投递给 strand 管理队列, 再由 strand 统一调度派发, 确保同一个 session 串行执行
    asio::strand<asio::io_context::executor_type> strand_;
};

}  // namespace network
