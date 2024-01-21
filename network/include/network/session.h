#pragma once

#include <boost/asio.hpp>

namespace network {

namespace asio = boost::asio;

using asio::ip::tcp;

class Session {
public:
    static constexpr size_t kMaxLength = 1024;

public:
    Session(asio::io_context& io_context) : socket_(io_context) {}

    tcp::socket& Socket() { return socket_; }

    void Start();

private:
    void HandleRead(const boost::system::error_code& error_code, size_t bytes_transferred);

    void HandleWrite(const boost::system::error_code& error_code);

private:
    tcp::socket socket_;
    char data_[kMaxLength];
};

}  // namespace network
