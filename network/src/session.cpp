#include "network/session.h"

#include <fmt/printf.h>

#include "network/logic_system.h"
#include "network/server.h"
#include "network/utils.h"

namespace network {

void Session::Start() {
    memset(data_, 0, kMaxLength);
    socket_.async_read_some(
        asio::buffer(data_, kMaxLength),
        [shared_this = shared_from_this()](const boost::system::error_code& error_code, size_t bytes_transferred) {
            shared_this->HandleRead(error_code, bytes_transferred);
        });
}

void Session::Send(const char* msg, size_t max_len, MsgId msg_id) {
    std::lock_guard<std::mutex> lock(send_lock_);
    size_t send_queue_size = send_queue_.size();
    if (send_queue_size > kMaxSendQueue) {
        fmt::println("[{}]: Send queue size = {} is full", __func__, send_queue_size);
        return;
    }
    send_queue_.emplace(std::make_unique<SendNode>(msg, max_len, msg_id));
    if (send_queue_size > 0) {
        return;
    }

    asio::async_write(socket_, asio::buffer(send_queue_.front()->data_, send_queue_.front()->total_len_),
                      [shared_this = shared_from_this()](const boost::system::error_code& error_code,
                                                         [[maybe_unused]] size_t bytes_transferred) {
                          shared_this->HandleWrite(error_code);
                      });
}

void Session::Send(const std::string& msg, MsgId msg_id) { Send(msg.data(), msg.size(), msg_id); }

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
            MsgHead head = MsgHead::ParseHead(recv_head_->data_);
            // 头部长度非法
            if (head.id > MsgId::kMaxId || head.length > kMaxLength) {
                fmt::println("[{}]: Head: {} is invalid", __func__, head);
                socket_.close();
                server_->DeleteSession(uuid_);
                return;
            }
            fmt::println("[{}]: Head: {}", __func__, head);
            recv_msg_ = std::make_unique<RecvNode>(head.length, head.id);

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
            // 投递数据
            LogicSystem::Instance().PostMsgToQueue(
                std::make_unique<LogicNode>(shared_from_this(), std::move(recv_msg_)));
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
