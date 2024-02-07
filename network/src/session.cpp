#include "network/session.h"

#include <fmt/printf.h>

#include "network/server.h"

namespace network {

void Session::Start() {
    memset(data_, 0, kMaxLength);
    socket_.async_read_some(
        asio::buffer(data_, kMaxLength),
        [shared_this = shared_from_this()](const boost::system::error_code& error_code, size_t bytes_transferred) {
            shared_this->HandleRead(error_code, bytes_transferred);
        });
}

void Session::HandleRead(const boost::system::error_code& error_code, [[maybe_unused]] size_t bytes_transferred) {
    if (!error_code) {
        fmt::println("[{}]: Server receive data {}", __func__, data_);

        asio::async_write(socket_, asio::buffer("hello client!"),
                          [shared_this = shared_from_this()](const boost::system::error_code& error_code,
                                                             [[maybe_unused]] std::size_t bytes_transferred) {
                              shared_this->HandleWrite(error_code);
                          });

        memset(data_, 0, kMaxLength);
        socket_.async_read_some(
            asio::buffer(data_, kMaxLength),
            [shared_this = shared_from_this()](const boost::system::error_code& error_code, size_t bytes_transferred) {
                shared_this->HandleRead(error_code, bytes_transferred);
            });
    } else {
        fmt::println("[{}]: Error code = {}. Message: {}", __func__, error_code.value(), error_code.message());

        // 关闭 socket 并取消所有挂起的异步操作
        socket_.close();

        // 从服务器中删除 session 引用
        server_->DeleteSession(uuid_);
    }
}

void Session::HandleWrite(const boost::system::error_code& error_code) {
    if (!error_code) {
        memset(data_, 0, kMaxLength);
        socket_.async_read_some(
            asio::buffer(data_, kMaxLength),
            [shared_this = shared_from_this()](const boost::system::error_code& error_code, size_t bytes_transferred) {
                shared_this->HandleRead(error_code, bytes_transferred);
            });
    } else {
        fmt::println("[{}]: Error code = {}. Message: {}", __func__, error_code.value(), error_code.message());

        // 关闭 socket 并取消所有挂起的异步操作
        socket_.close();

        // 从服务器中删除 session 引用
        server_->DeleteSession(uuid_);
    }
}

}  // namespace network
