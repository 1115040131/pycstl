#include "chat/server/chat_server/csession.h"

#include "chat/server/chat_server/cserver.h"
#include "chat/server/chat_server/logic_system.h"
#include "chat/server/common/utils.h"

namespace pyc {
namespace chat {

CSession::CSession(boost::asio::io_context& io_context, CServer* server)
    : socket_(io_context), server_(server), session_id_(generateUniqueString()) {}

void CSession::Start() { AsyncReadHead(kHeadLength); }

void CSession::Close() {
    if (is_closed_) {
        return;
    }

    is_closed_ = true;
    boost::system::error_code ec;
    socket_.shutdown(tcp::socket::shutdown_both, ec);
    socket_.close(ec);
    server_->ClearSession(session_id_);
}

void CSession::asyncReadFull(size_t max_length,
                             std::function<void(const boost::system::error_code&, size_t)> handler) {
    asyncReadLen(0, max_length, handler);
}

void CSession::asyncReadLen(size_t read_len, size_t total_len,
                            std::function<void(const boost::system::error_code&, size_t)> handler) {
    socket_.async_read_some(boost::asio::buffer(data_ + read_len, total_len - read_len),
                            [read_len, total_len, handler, self = shared_from_this()](
                                const boost::system::error_code& ec, size_t bytes_transfered) {
                                // 出现错误
                                if (ec) {
                                    handler(ec, read_len + bytes_transfered);
                                    return;
                                }
                                // 读取完毕
                                if (read_len + bytes_transfered >= total_len) {
                                    handler(ec, read_len + bytes_transfered);
                                    return;
                                }

                                // 继续读取
                                self->asyncReadLen(read_len + bytes_transfered, total_len, handler);
                            });
}

void CSession::AsyncReadHead(size_t total_len) {
    asyncReadFull(total_len, [this, self = shared_from_this(), total_len](const boost::system::error_code& ec,
                                                                          size_t bytes_transfered) {
        try {
            if (ec) {
                PYC_LOG_ERROR("Error code = {}. Message: {}", ec.value(), ec.message());
                Close();

                return;
            }

            if (bytes_transfered < total_len) {
                PYC_LOG_ERROR("Read {} not match {}", bytes_transfered, total_len);
                Close();
                return;
            }

            MsgHead msg_head = MsgHead::ParseHead(data_);
            // id 非法
            if (msg_head.id == ReqId::kInvalid) {
                PYC_LOG_ERROR("Invalid head id: {}, length: {}", static_cast<int>(msg_head.id), msg_head.length);
                Close();
                return;
            }

            recv_node_ = std::make_unique<RecvNode>(msg_head.length, msg_head.id);
            AsyncReadBody(msg_head.length);
        } catch (const std::exception& e) {
            PYC_LOG_ERROR("{}", e.what());
        }
    });
}

void CSession::AsyncReadBody(size_t total_len) {
    asyncReadFull(total_len, [this, self = shared_from_this(), total_len](const boost::system::error_code& ec,
                                                                          size_t bytes_transfered) {
        try {
            if (ec) {
                PYC_LOG_ERROR("Error code = {}. Message: {}", ec.value(), ec.message());
                Close();
                return;
            }

            if (bytes_transfered < total_len) {
                PYC_LOG_ERROR("Read {} not match {}", bytes_transfered, total_len);
                Close();
                return;
            }

            recv_node_->Copy(data_, total_len);
            // 投递数据
            LogicSystem::GetInstance().PostMsgToQueue(
                std::make_unique<LogicNode>(shared_from_this(), std::move(recv_node_)));
            // 继续读取
            AsyncReadHead(kHeadLength);
        } catch (const std::exception& e) {
            PYC_LOG_ERROR("{}", e.what());
        }
    });
}

void CSession::Send(const char* msg, size_t max_len, ReqId msg_id) {
    std::lock_guard<std::mutex> lock(send_lock_);
    size_t send_queue_size = send_queue_.size();
    if (send_queue_size > kMaxSendQueue) {
        PYC_LOG_WARN("Send queue size = {} is full", send_queue_size);
        return;
    }
    send_queue_.emplace(std::make_unique<SendNode>(msg, max_len, msg_id));
    if (send_queue_size > 0) {
        return;
    }

    auto& send_node = send_queue_.front();
    boost::asio::async_write(
        socket_, boost::asio::buffer(send_node->Data(), send_node->Size()),
        [self = shared_from_this()](const boost::system::error_code& ec, std::size_t) { self->HandleWrite(ec); });
}

void CSession::Send(const std::string& msg, ReqId msg_id) { Send(msg.data(), msg.size(), msg_id); }

void CSession::HandleWrite(const boost::system::error_code& ec) {
    try {
        if (ec) {
            PYC_LOG_ERROR("Error code = {}. Message: {}", ec.value(), ec.message());
            Close();
            return;
        }

        std::lock_guard<std::mutex> lock(send_lock_);
        send_queue_.pop();
        if (!send_queue_.empty()) {
            auto& send_node = send_queue_.front();
            boost::asio::async_write(socket_, boost::asio::buffer(send_node->Data(), send_node->Size()),
                                     [self = shared_from_this()](const boost::system::error_code& ec,
                                                                 std::size_t) { self->HandleWrite(ec); });
        }
    } catch (const std::exception& e) {
        PYC_LOG_ERROR("{}", e.what());
    }
}

}  // namespace chat
}  // namespace pyc
