#pragma once

#include <memory>
#include <mutex>

#include <queue>

#include "chat/server/chat_server/define.h"
#include "chat/server/chat_server/msg_node.h"

namespace pyc {
namespace chat {

class CServer;

class CSession : public std::enable_shared_from_this<CSession> {
public:
    CSession(boost::asio::io_context& io_context, CServer* server);

    tcp::socket& GetSocket() { return socket_; }

    const std::string& GetUuid() const { return uuid_; }

    // 开始异步读取数据
    void Start();

    // 往发送队列添加发送数据并异步发送
    void Send(const char* msg, std::size_t max_len, ReqId msg_id);
    void Send(const std::string& msg, ReqId msg_id);

private:
    // 关闭连接
    void Close();

    // 读取完整长度
    void asyncReadFull(size_t max_length, std::function<void(const boost::system::error_code&, size_t)> handler);

    // 读取指定字节数
    void asyncReadLen(size_t read_len, size_t total_len,
                      std::function<void(const boost::system::error_code&, size_t)> handler);

    // 接收头部
    void AsyncReadHead(size_t total_len);

    // 接收数据
    void AsyncReadBody(size_t total_len);

    // 写回调
    void HandleWrite(const boost::system::error_code& error_code);

private:
    tcp::socket socket_;
    CServer* server_;
    std::string uuid_;
    bool is_closed_ = false;

    char data_[kMaxLength];                // 接收数据缓冲区
    std::unique_ptr<RecvNode> recv_node_;  // 接收数据节点

    static constexpr std::size_t kMaxSendQueue = 1000;  // 发送队列最大长度
    std::queue<std::unique_ptr<SendNode>> send_queue_;  // 发送队列
    std::mutex send_lock_;                              // 发送队列锁
};

}  // namespace chat
}  // namespace pyc