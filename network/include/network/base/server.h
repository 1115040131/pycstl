#pragma once

#include <memory>
#include <mutex>
#include <unordered_map>

#include <boost/asio.hpp>

namespace network {

namespace asio = boost::asio;
using asio::ip::tcp;

class Session;

class Server {
public:
    Server(asio::io_context& io_context, unsigned short port_num);

    virtual ~Server() = default;

    // 使用 IOServicePool 或者 ThreadPool 这里的行为会不一样
    virtual void StartAccept() = 0;

    void DeleteSession(const std::string& uuid);

protected:
    void HandleAccept(const std::shared_ptr<Session>& session, const boost::system::error_code& error_code);

protected:
    asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    std::unordered_map<std::string, std::shared_ptr<Session>> sessions_{};
    std::mutex mutex_{};
};

}  // namespace network
