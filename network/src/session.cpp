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
    if (error_code) {
        fmt::println("[{}]: Error code = {}. Message: {}", __func__, error_code.value(), error_code.message());
        // 关闭 socket 并取消所有挂起的异步操作
        socket_.close();
        // 从服务器中删除 session 引用
        server_->DeleteSession(uuid_);
        return;
    }

    // 打印接收数据
    fmt::print("[{}]: Server receive raw data: ", __func__);
    PrintBuffer(data_, bytes_transferred);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 已经处理的字节
    size_t copy_len = 0;
    while (bytes_transferred > 0) {
        if (!is_head_parsed_) {
            size_t head_remain = kHeadLength - recv_head_->cur_len_;  // 头部剩余长度

            // 收到的数据不足以解析出头部
            if (bytes_transferred < head_remain) {
                memcpy(recv_head_->data_ + recv_head_->cur_len_, data_ + copy_len, bytes_transferred);
                recv_head_->cur_len_ += bytes_transferred;
                break;
            }

            // 收到的数据足够解析出头部
            memcpy(recv_head_->data_ + recv_head_->cur_len_, data_ + copy_len, head_remain);
            // 更新已处理的字节数
            copy_len += head_remain;
            bytes_transferred -= head_remain;
            // 获取头部数据
            MsgSizeType data_len;
            memcpy(&data_len, recv_head_->data_, kHeadLength);
            // 头部长度非法
            if (data_len > kMaxLength) {
                fmt::println("[{}]: Head length = {} is invalid", __func__, data_len);
                socket_.close();
                server_->DeleteSession(uuid_);
                return;
            }
            fmt::println("[{}]: Head length = {}", __func__, data_len);
            recv_msg_ = std::make_unique<MsgNode>(data_len);

            is_head_parsed_ = true;
        } else {
            size_t data_len = recv_msg_->total_len_ - recv_msg_->cur_len_;  // 数据剩余长度

            // 消息的长度小于头部长度, 数据未收全
            if (bytes_transferred < data_len) {
                memcpy(recv_msg_->data_ + recv_msg_->cur_len_, data_ + copy_len, bytes_transferred);
                recv_msg_->cur_len_ += bytes_transferred;
                break;
            }

            // 消息的长度大于等于头部长度, 数据已收全
            memcpy(recv_msg_->data_ + recv_msg_->cur_len_, data_ + copy_len, data_len);
            // 更新已处理的字节数
            copy_len += data_len;
            bytes_transferred -= data_len;
            fmt::println("[{}]: Server receive data {}", __func__, recv_msg_->data_);
            // 发送数据
            Send(recv_msg_->data_, recv_msg_->total_len_);
            // 处理新的数据块
            is_head_parsed_ = false;
            recv_head_->Clear();
        }
    }

    // 继续轮询剩余未处理数据
    memset(data_, 0, kMaxLength);
    socket_.async_read_some(
        asio::buffer(data_, kMaxLength),
        [shared_this = shared_from_this()](const boost::system::error_code& error_code, size_t bytes_transferred) {
            shared_this->HandleRead(error_code, bytes_transferred);
        });
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
        socket_.close();
        server_->DeleteSession(uuid_);
    }
}

void Session::PrintBuffer(const char* buffer, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        fmt::print("{:02x} ", buffer[i]);
    }
    fmt::print("\n");
}

}  // namespace network
