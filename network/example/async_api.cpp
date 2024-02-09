#include "example/async_api.h"

#include <fmt/printf.h>

namespace network {

void WriteErrSession::WriteToSocket(const std::string& msg) {
    send_node_ = std::make_shared<MsgNode>(msg);
    socket_->async_write_some(asio::buffer(send_node_->msg_),
                              [this](const boost::system::error_code& error_code, size_t bytes_transferred) {
                                  WriteCallBack(error_code, bytes_transferred, send_node_);
                              });
}

void WriteErrSession::WriteCallBack(const boost::system::error_code& error_code, size_t bytes_transferred,
                                    std::shared_ptr<MsgNode> msg_node) {
    if (error_code.value() != 0) {
        fmt::println("write failed! Error code = {}. Message: {}", error_code.value(), error_code.message());
        return;
    }

    if (bytes_transferred + msg_node->cur_len_ < msg_node->total_len_) {
        send_node_->cur_len_ += bytes_transferred;
        socket_->async_write_some(asio::buffer(send_node_->msg_.data() + send_node_->cur_len_,
                                               send_node_->total_len_ - send_node_->cur_len_),
                                  [this](const boost::system::error_code& error_code, size_t bytes_transferred) {
                                      WriteCallBack(error_code, bytes_transferred, send_node_);
                                  });
    }
}

void WriteSession::WriteToSocket(const std::string& msg) {
    send_queue_.emplace(std::make_shared<MsgNode>(msg));
    if (send_pending_) {
        return;
    }

    send_pending_ = true;

    socket_->async_write_some(asio::buffer(msg),
                              [this](const boost::system::error_code& error_code, size_t bytes_transferred) {
                                  WriteCallBack(error_code, bytes_transferred);
                              });
}

void WriteSession::WriteCallBack(const boost::system::error_code& error_code, size_t bytes_transferred) {
    if (error_code.value() != 0) {
        fmt::println("write failed! Error code = {}. Message: {}", error_code.value(), error_code.message());
        send_pending_ = true;
        return;
    }

    auto& send_node = send_queue_.front();
    send_node->cur_len_ += bytes_transferred;
    if (send_node->cur_len_ < send_node->total_len_) {
        socket_->async_write_some(asio::buffer(send_node->msg_.data() + send_node->cur_len_,
                                               send_node->total_len_ - send_node->cur_len_),
                                  [this](const boost::system::error_code& error_code, size_t bytes_transferred) {
                                      WriteCallBack(error_code, bytes_transferred);
                                  });

        return;
    }

    send_queue_.pop();
    if (send_queue_.empty()) {
        send_pending_ = false;
    } else {
        auto& send_node = send_queue_.front();
        socket_->async_write_some(asio::buffer(send_node->msg_),
                                  [this](const boost::system::error_code& error_code, size_t bytes_transferred) {
                                      WriteCallBack(error_code, bytes_transferred);
                                  });
    }
}

void WriteAllSession::WriteToSocket(const std::string& msg) {
    send_queue_.emplace(std::make_shared<MsgNode>(msg));
    if (send_pending_) {
        return;
    }

    send_pending_ = true;

    socket_->async_send(asio::buffer(msg),
                        [this](const boost::system::error_code& error_code, size_t bytes_transferred) {
                            WriteCallBack(error_code, bytes_transferred);
                        });
}

void WriteAllSession::WriteCallBack(const boost::system::error_code& error_code,
                                    [[maybe_unused]] size_t bytes_transferred) {
    if (error_code.value() != 0) {
        fmt::println("write failed! Error code = {}. Message: {}", error_code.value(), error_code.message());
        send_pending_ = true;
        return;
    }

    send_queue_.pop();
    if (send_queue_.empty()) {
        send_pending_ = false;
    } else {
        auto& send_node = send_queue_.front();
        socket_->async_send(asio::buffer(send_node->msg_),
                            [this](const boost::system::error_code& error_code, size_t bytes_transferred) {
                                WriteCallBack(error_code, bytes_transferred);
                            });
    }
}

void ReadSession::ReadFromSocket() {
    if (recv_pending_) {
        return;
    }

    recv_pending_ = true;
    recv_node_ = std::make_shared<MsgNode>(kRecvSize);
    socket_->async_read_some(asio::buffer(recv_node_->msg_, recv_node_->total_len_),
                             [this](const boost::system::error_code& error_code, size_t bytes_transferred) {
                                 ReadCallBack(error_code, bytes_transferred);
                             });
}

void ReadSession::ReadCallBack(const boost::system::error_code& error_code, size_t bytes_transferred) {
    if (error_code.value() != 0) {
        fmt::println("write failed! Error code = {}. Message: {}", error_code.value(), error_code.message());
        recv_pending_ = true;
        return;
    }

    recv_node_->cur_len_ += bytes_transferred;
    if (recv_node_->cur_len_ < recv_node_->total_len_) {
        socket_->async_read_some(asio::buffer(recv_node_->msg_.data() + recv_node_->cur_len_,
                                              recv_node_->total_len_ - recv_node_->cur_len_),
                                 [this](const boost::system::error_code& error_code, size_t bytes_transferred) {
                                     ReadCallBack(error_code, bytes_transferred);
                                 });
        return;
    }

    recv_pending_ = false;
}

void ReadAllSession::ReadFromSocket() {
    if (recv_pending_) {
        return;
    }

    recv_pending_ = true;
    recv_node_ = std::make_shared<MsgNode>(kRecvSize);
    socket_->async_receive(asio::buffer(recv_node_->msg_, recv_node_->total_len_),
                           [this](const boost::system::error_code& error_code, size_t bytes_transferred) {
                               ReadCallBack(error_code, bytes_transferred);
                           });
}

void ReadAllSession::ReadCallBack(const boost::system::error_code& error_code, size_t bytes_transferred) {
    if (error_code.value() != 0) {
        fmt::println("write failed! Error code = {}. Message: {}", error_code.value(), error_code.message());
        recv_pending_ = true;
        return;
    }

    recv_node_->cur_len_ += bytes_transferred;
    recv_pending_ = false;
}

}  // namespace network
