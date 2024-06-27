#pragma once

#include <memory>
#include <mutex>
#include <queue>
#include <string>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

namespace network {

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(asio::io_context& io_context);

    tcp::socket& Socket() { return beast::get_lowest_layer(*websocket_).socket(); }

    const std::string& GetUuid() const { return uuid_; }

    /// @brief 开启 websocket 连接
    void StartAccept();

private:
    void HandleAccept(const beast::error_code& error_code);

    void AsyncRead();

    void HandleRead(const beast::error_code& error_code);

    void AsyncSend(std::string msg);

    void AsyncWrite();

    void HandleSend(const beast::error_code& error_code);

private:
    asio::io_context& io_context_;
    std::unique_ptr<websocket::stream<beast::tcp_stream>> websocket_;
    std::string uuid_{};
    beast::flat_buffer recv_buffer_{};
    std::queue<std::string> send_queue_{};
    std::mutex send_mutex_{};
};

}  // namespace network