module;

#include <memory>

#include <boost/asio.hpp>

export module chat.server.gate_server:cserver;

import chat.server.gate_server.define;

export namespace pyc {
namespace chat {

class CServer : public std::enable_shared_from_this<CServer> {
public:
    CServer(unsigned short port);

    void Start();

private:
    tcp::acceptor acceptor_;
};

}  // namespace chat
}  // namespace pyc
