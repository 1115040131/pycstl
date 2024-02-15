#include "network/io_service_pool/io_service_pool_session.h"

#include <fmt/printf.h>

namespace network {

IOServicePoolSession::~IOServicePoolSession() {
    fmt::println("[{}]: IOServicePoolSession {} destruct uuid = {}", __func__, reinterpret_cast<uint64_t>(this),
                 uuid_);
}

void IOServicePoolSession::Start() {
    fmt::println("[{}]: IOServicePoolSession {} start uuid = {}", __func__, reinterpret_cast<uint64_t>(this),
                 uuid_);
    AsyncRead();
}

void IOServicePoolSession::AsyncRead() {
    ::memset(data_, 0, kMaxLength);
    socket_.async_read_some(asio::buffer(data_, kMaxLength),
                            [shared_this = std::static_pointer_cast<IOServicePoolSession>(shared_from_this())](
                                const boost::system::error_code& error_code, size_t bytes_transferred) {
                                shared_this->HandleRead(error_code, bytes_transferred);
                            });
}

void IOServicePoolSession::AsyncWrite() {
    asio::async_write(
        socket_, asio::buffer(send_queue_.front()->Data(), send_queue_.front()->Size()),
        [shared_this = std::static_pointer_cast<IOServicePoolSession>(shared_from_this())](
            const boost::system::error_code& error_code, size_t) { shared_this->HandleWrite(error_code); });
}

}  // namespace network
