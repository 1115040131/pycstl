#pragma once

#include <memory>

#include "chat/server/gate_server/define.h"

namespace pyc {
namespace chat {

class CServer : public std::enable_shared_from_this<CServer> {
public:
    CServer(asio::io_context& io_context, unsigned short port);

    void Start();

private:
    asio::io_context& io_context_;
    tcp::acceptor acceptor_;
};

}  // namespace chat
}  // namespace pyc
