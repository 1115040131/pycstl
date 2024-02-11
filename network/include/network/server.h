#pragma once

#include <memory>
#include <unordered_map>

#include "network/session.h"

namespace network {

class Server {
public:
    Server(asio::io_context& io_context, unsigned short port_num);

    void DeleteSession(const std::string& uuid);

private:
    void StartAccept();

    void HandleAccept(const std::shared_ptr<Session>& session, const boost::system::error_code& error_code);

private:
    asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
};

}  // namespace network
