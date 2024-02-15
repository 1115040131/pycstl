#pragma once

#include <memory>
#include <queue>

#include <boost/asio.hpp>

namespace network {

namespace asio = boost::asio;

constexpr std::size_t kRecvSize = 1024;

class MsgNode {
private:
    friend class WriteErrSession;
    friend class WriteSession;
    friend class WriteAllSession;
    friend class ReadSession;
    friend class ReadAllSession;

public:
    MsgNode(std::string msg) : msg_(std::move(msg)), total_len_(msg.size()) {}

    explicit MsgNode(std::size_t total_len) : total_len_(total_len) {}

private:
    std::size_t total_len_;
    std::size_t cur_len_ = 0;
    std::string msg_;
};

class Session {
public:
    explicit Session(const std::shared_ptr<asio::ip::tcp::socket>& socket) : socket_(socket) {}

    void Connect(const asio::ip::tcp::endpoint& endpoint) { socket_->connect(endpoint); }

protected:
    std::shared_ptr<asio::ip::tcp::socket> socket_;
};

// 在等待 WriteCallBack 执行的时候又一次调用 WriteToSocket 会发生错误
class WriteErrSession : public Session {
public:
    explicit WriteErrSession(const std::shared_ptr<asio::ip::tcp::socket>& socket) : Session(socket) {}

    void WriteToSocket(const std::string& msg);

private:
    void WriteCallBack(const boost::system::error_code& error_code, std::size_t bytes_transferred,
                       std::shared_ptr<MsgNode> node);

private:
    std::shared_ptr<MsgNode> send_node_;
};

class WriteSession : public Session {
public:
    explicit WriteSession(const std::shared_ptr<asio::ip::tcp::socket>& socket) : Session(socket) {}

    void WriteToSocket(const std::string& msg);

private:
    void WriteCallBack(const boost::system::error_code& error_code, std::size_t bytes_transferred);

private:
    std::queue<std::shared_ptr<MsgNode>> send_queue_;
    bool send_pending_ = false;  // 有未发完的数据
};

// send 封装了多次的 write_some 避免频繁回调
class WriteAllSession : public Session {
public:
    explicit WriteAllSession(const std::shared_ptr<asio::ip::tcp::socket>& socket) : Session(socket) {}

    void WriteToSocket(const std::string& msg);

private:
    void WriteCallBack(const boost::system::error_code& error_code, std::size_t bytes_transferred);

private:
    std::queue<std::shared_ptr<MsgNode>> send_queue_;
    bool send_pending_ = false;  // 有未发完的数据
};

class ReadSession : public Session {
public:
    explicit ReadSession(const std::shared_ptr<asio::ip::tcp::socket>& socket) : Session(socket) {}

    void ReadFromSocket();

private:
    void ReadCallBack(const boost::system::error_code& error_code, std::size_t bytes_transferred);

private:
    std::shared_ptr<MsgNode> recv_node_ = std::make_shared<MsgNode>(kRecvSize);
    bool recv_pending_ = false;  // 有未收完的数据
};

class ReadAllSession : public Session {
public:
    explicit ReadAllSession(const std::shared_ptr<asio::ip::tcp::socket>& socket) : Session(socket) {}

    void ReadFromSocket();

private:
    void ReadCallBack(const boost::system::error_code& error_code, std::size_t bytes_transferred);

private:
    std::shared_ptr<MsgNode> recv_node_ = std::make_shared<MsgNode>(kRecvSize);
    bool recv_pending_ = false;  // 有未收完的数据
};

}  // namespace network
