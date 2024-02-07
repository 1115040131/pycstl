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

void Session::Send(char* msg, size_t max_len) {
    bool pending = false;
    std::lock_guard<std::mutex> lock(send_lock_);
    if (!send_queue_.empty()) {
        pending = true;
    }
    send_queue_.emplace(std::make_unique<MsgNode>(msg, max_len));
    if (pending) {
        return;
    }

    asio::async_write(socket_, asio::buffer(send_queue_.front()->data_, send_queue_.front()->total_len_),
                      [shared_this = shared_from_this()](const boost::system::error_code& error_code,
                                                         [[maybe_unused]] size_t bytes_transferred) {
                          shared_this->HandleWrite(error_code);
                      });
}

void Session::HandleRead(const boost::system::error_code& error_code, size_t bytes_transferred) {
    if (!error_code) {
        fmt::println("[{}]: Server receive data {}", __func__, data_);

        Send(data_, bytes_transferred);
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
        std::lock_guard<std::mutex> lock(send_lock_);
        send_queue_.pop();
        if (!send_queue_.empty()) {
            asio::async_write(socket_, asio::buffer(send_queue_.front()->data_, send_queue_.front()->total_len_),
                              [shared_this = shared_from_this()](const boost::system::error_code& error_code,
                                                                 [[maybe_unused]] size_t bytes_transferred) {
                                  shared_this->HandleWrite(error_code);
                              });
        }
    } else {
        fmt::println("[{}]: Error code = {}. Message: {}", __func__, error_code.value(), error_code.message());

        // 关闭 socket 并取消所有挂起的异步操作
        socket_.close();

        // 从服务器中删除 session 引用
        server_->DeleteSession(uuid_);
    }
}

}  // namespace network
