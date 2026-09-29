module;

#include <boost/asio.hpp>

export module network.websocket_server;

import common.noncopyable;

export namespace network {

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

class WebsocketServer : public pyc::Noncopyable {
public:
    WebsocketServer(asio::io_context& io_context, unsigned short port);

    /// @brief 开启 tcp 连接
    void StartAccept();

private:
    asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

}  // namespace network
