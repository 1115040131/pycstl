#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <string>

#include <boost/asio.hpp>

#include "network/msg_node.h"

namespace network {

namespace asio = boost::asio;

using asio::ip::tcp;

class Server;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(asio::io_context& io_context, Server* server);

    virtual ~Session();

    tcp::socket& Socket() { return socket_; }

    const std::string& GetUuid() const { return uuid_; }

    /// @brief 开始异步读取数据
    void Start();

    /// @brief 往发送队列添加发送数据
    void Send(const char* msg, size_t max_len, MsgId msg_id);
    void Send(const std::string& msg, MsgId msg_id);

protected:
    /// @brief 关闭连接
    void Stop();

    /// @brief 异步读取数据
    virtual void AsyncRead() = 0;

    /// @brief 异步写入数据
    virtual void AsyncWrite() = 0;

    /// @brief 异步读回调
    void HandleRead(const boost::system::error_code& error_code, size_t bytes_transferred);

    /// @brief 异步写回调
    void HandleWrite(const boost::system::error_code& error_code);

    /// @brief 打印缓冲区数据
    void PrintBuffer(const char* buffer, size_t len);

protected:
    tcp::socket socket_;
    Server* server_;
    std::string uuid_;


    char data_[kMaxLength];  // 接收数据缓冲区

    std::queue<std::unique_ptr<SendNode>> send_queue_;  // 发送队列
    std::mutex send_lock_;                              // 发送队列锁

    bool is_head_parsed_ = false;  // 是否已经解析出头部
    std::unique_ptr<RecvNode> recv_head_ = std::make_unique<RecvNode>(kHeadLength, MsgId::kMaxId);  // 头部数据
    std::unique_ptr<RecvNode> recv_msg_;                                                            // 接收数据
};

}  // namespace network
