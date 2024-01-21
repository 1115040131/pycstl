#pragma once

#include "network/session.h"

namespace network {

class Server {
public:
    Server(asio::io_context& io_context, unsigned short port_num);

private:
    void StartAccept();

    void HandleAccept(Session* session, const boost::system::error_code& error_code);

private:
    asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

}  // namespace network
