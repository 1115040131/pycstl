#include "network/websocket/connection.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fmt/core.h>

#include "network/websocket/connection_manager.h"

namespace network {

Connection::Connection(asio::io_context& io_context)
    : io_context_(io_context),
      websocket_(std::make_unique<websocket::stream<beast::tcp_stream>>(asio::make_strand(io_context))) {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    uuid_ = boost::uuids::to_string(uuid);
}

void Connection::StartAccept() {
    websocket_->async_accept([shared_this = shared_from_this()](const beast::error_code& error_code) {
        shared_this->HandleAccept(error_code);
    });
}

void Connection::HandleAccept(const beast::error_code& error_code) {
    if (error_code) {
        fmt::println("[{}]: Error code: {}. Message: {}", __func__, error_code.value(), error_code.message());
        return;
    }

    ConnectionManager::GetInstance().AddConnection(shared_from_this());
    AsyncRead();
}

void Connection::AsyncRead() {
    websocket_->async_read(recv_buffer_,
                           [shared_this = shared_from_this()](const beast::error_code& error_code, std::size_t) {
                               shared_this->HandleRead(error_code);
                           });
}

void Connection::HandleRead(const beast::error_code& error_code) {
    if (error_code) {
        fmt::println("[{}]: Error code: {}. Message: {}", __func__, error_code.value(), error_code.message());
        ConnectionManager::GetInstance().RemoveConnection(GetUuid());
        return;
    }

    websocket_->text(websocket_->got_text());
    std::string recv_data = beast::buffers_to_string(recv_buffer_.data());
    recv_buffer_.consume(recv_buffer_.size());  // 清空
    fmt::println("[{}]: Received data: {}", __func__, recv_data);

    AsyncSend(std::move(recv_data));
    AsyncRead();
}

void Connection::AsyncSend(std::string msg) {
    std::lock_guard<std::mutex> lock(send_mutex_);
    bool is_sending = !send_queue_.empty();
    send_queue_.push(std::move(msg));
    if (is_sending) {
        return;
    }
    AsyncWrite();
}

void Connection::AsyncWrite() {
    websocket_->async_write(asio::buffer(send_queue_.front()),
                            [shared_this = shared_from_this()](const beast::error_code& error_code, std::size_t) {
                                shared_this->HandleSend(error_code);
                            });
}

void Connection::HandleSend(const beast::error_code& error_code) {
    if (error_code) {
        fmt::println("[{}]: Error code: {}. Message: {}", __func__, error_code.value(), error_code.message());
        ConnectionManager::GetInstance().RemoveConnection(GetUuid());
        return;
    }

    std::lock_guard<std::mutex> lock(send_mutex_);
    send_queue_.pop();
    if (send_queue_.empty()) {
        return;
    }
    AsyncWrite();
}

}  // namespace network
