#pragma once

#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <fmt/printf.h>

namespace network {

namespace asio = boost::asio;

using asio::ip::tcp;

class Server;

class Session : public std::enable_shared_from_this<Session> {
public:
    static constexpr size_t kMaxLength = 1024;

public:
    Session(asio::io_context& io_context, Server* server) : socket_(io_context), server_(server) {
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        uuid_ = boost::uuids::to_string(uuid);
    }

    ~Session() {
        fmt::println("[{}]: Session {} destruct uuid = {}", __func__, reinterpret_cast<uint64_t>(this), uuid_);
    }

    tcp::socket& Socket() { return socket_; }

    void Start();

    const std::string& GetUuid() const { return uuid_; }

private:
    void HandleRead(const boost::system::error_code& error_code, size_t bytes_transferred);

    void HandleWrite(const boost::system::error_code& error_code);

private:
    tcp::socket socket_;
    char data_[kMaxLength];
    Server* server_;
    std::string uuid_;
};

}  // namespace network
