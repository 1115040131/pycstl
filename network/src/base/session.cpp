#include "network/base/session.h"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "common/utils.h"
#include "network/base/server.h"
#include "network/logic_system.h"
#include "network/utils.h"

namespace network {

Session::Session(asio::io_context& io_context, Server* server) : socket_(io_context), server_(server) {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    uuid_ = boost::uuids::to_string(uuid);
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

    AsyncWrite();
}

void Session::Send(const std::string& msg, MsgId msg_id) { Send(msg.data(), msg.size(), msg_id); }

void Session::Stop() {
    // 关闭 socket 并取消所有挂起的异步操作
    socket_.close();
    // 从服务器中删除 session 引用
    server_->DeleteSession(uuid_);
}

void Session::HandleRead(const boost::system::error_code& error_code, size_t bytes_transferred) {
    if (error_code) {
        fmt::println("[{}]: Error code = {}. Message: {}", __func__, error_code.value(), error_code.message());
        Stop();
        return;
    }

    // For debug 打印接收数据
    // fmt::print("[{}]: Server receive raw data: ", __func__);
    // PrintBuffer(data_, bytes_transferred);
    // std::this_thread::sleep_for(std::chrono::seconds(2));

    // 已经处理的字节
    size_t offset = 0;
    while (bytes_transferred > 0) {
        if (!is_head_parsed_) {
            size_t copy_len = recv_head_->Copy(data_ + offset, bytes_transferred);

            // 收到的数据不足以解析出头部
            if (recv_head_->Remain() > 0) {
                break;
            }

            // 更新已处理的字节数
            offset += copy_len;
            bytes_transferred -= copy_len;
            // 获取头部数据
            MsgHead head = MsgHead::ParseHead(recv_head_->Data());
            // 头部长度非法
            if (head.id > MsgId::kMaxId || head.length > kMaxLength) {
                fmt::println("[{}]: Server receive valid head: {}", __func__, head);
                Stop();
                return;
            }
            fmt::println("[{}]: Server receive head: {}", __func__, head);
            recv_msg_ = std::make_unique<RecvNode>(head.length, head.id);

            is_head_parsed_ = true;
        } else {
            size_t copy_len = recv_msg_->Copy(data_ + offset, bytes_transferred);

            // 消息的长度小于头部长度, 数据未收全
            if (recv_msg_->Remain() > 0) {
                break;
            }

            // 更新已处理的字节数
            offset += copy_len;
            bytes_transferred -= copy_len;
            // 投递数据
            LogicSystem::Instance().PostMsgToQueue(
                std::make_unique<LogicNode>(shared_from_this(), std::move(recv_msg_)));
            // 处理新的数据块
            is_head_parsed_ = false;
            recv_head_->Clear();
        }
    }

    // 继续轮询剩余未处理数据
    AsyncRead();
}

void Session::HandleWrite(const boost::system::error_code& error_code) {
    if (error_code) {
        fmt::println("[{}]: Error code = {}. Message: {}", __func__, error_code.value(), error_code.message());
        Stop();
        return;
    }

    std::lock_guard<std::mutex> lock(send_lock_);
    send_queue_.pop();
    if (!send_queue_.empty()) {
        AsyncWrite();
    }
}

void Session::PrintBuffer(const char* buffer, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        fmt::print("{:02x} ", buffer[i]);
    }
    fmt::print("\n");
}

}  // namespace network
