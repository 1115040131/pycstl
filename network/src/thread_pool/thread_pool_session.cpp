#include "network/thread_pool/thread_pool_session.h"

#include <fmt/printf.h>

namespace network {

ThreadPoolSession::ThreadPoolSession(asio::io_context& io_context, Server* server)
    : Session(io_context, server), strand_(io_context.get_executor()) {}

ThreadPoolSession::~ThreadPoolSession() {
    fmt::println("[{}]: ThreadPoolSession {} destruct uuid = {}", __func__, reinterpret_cast<uint64_t>(this),
                 uuid_);
}

void ThreadPoolSession::Start() {
    fmt::println("[{}]: ThreadPoolSession {} start uuid = {}", __func__, reinterpret_cast<uint64_t>(this), uuid_);
    AsyncRead();
}

void ThreadPoolSession::AsyncRead() {
    ::memset(data_, 0, kMaxLength);
    socket_.async_read_some(
        asio::buffer(data_, kMaxLength),
        asio::bind_executor(strand_,
                            [shared_this = std::static_pointer_cast<ThreadPoolSession>(shared_from_this())](
                                const boost::system::error_code& error_code, std::size_t bytes_transferred) {
                                shared_this->HandleRead(error_code, bytes_transferred);
                            }));
}

void ThreadPoolSession::AsyncWrite() {
    asio::async_write(socket_, asio::buffer(send_queue_.front()->Data(), send_queue_.front()->Size()),
                      asio::bind_executor(
                          strand_, [shared_this = std::static_pointer_cast<ThreadPoolSession>(shared_from_this())](
                                       const boost::system::error_code& error_code, std::size_t) {
                              shared_this->HandleWrite(error_code);
                          }));
}

}  // namespace network
