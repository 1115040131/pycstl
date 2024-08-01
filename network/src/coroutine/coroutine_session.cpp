#include "network/coroutine/coroutine_session.h"

#include <fmt/base.h>

namespace network {

CoroutineSession::~CoroutineSession() {
    fmt::println("[{}]: CoroutineSession {} destruct uuid = {}", __func__, reinterpret_cast<uint64_t>(this),
                 uuid_);
}

void CoroutineSession::Start() {
    fmt::println("[{}]: CoroutineSession {} start uuid = {}", __func__, reinterpret_cast<uint64_t>(this), uuid_);
    // 开启协程接收
    asio::co_spawn(
        io_context_,
        [shared_this = std::static_pointer_cast<CoroutineSession>(shared_from_this())]() -> asio::awaitable<void> {
            co_await shared_this->CoawaitRead();
        },
        asio::detached);
}

asio::awaitable<void> CoroutineSession::CoawaitRead() {
    try {
        while (is_stop_.load() == false) {
            std::size_t recv_size =
                co_await socket_.async_read_some(asio::buffer(data_, kMaxLength), asio::use_awaitable);

            ParseBuffer(recv_size);
        }
    } catch (const std::exception& e) {
        fmt::println(stderr, "[{}]: exception: {}", __func__, e.what());
        Stop();
    }
}

void CoroutineSession::AsyncWrite() {
    asio::async_write(
        socket_, asio::buffer(send_queue_.front()->Data(), send_queue_.front()->Size()),
        [shared_this = std::static_pointer_cast<CoroutineSession>(shared_from_this())](
            const boost::system::error_code& error_code, std::size_t) { shared_this->HandleWrite(error_code); });
}

}  // namespace network
