#include "gate_server/cserver.h"

#include "gate_server/http_connection.h"

namespace pyc {
namespace chat {

CServer::CServer(asio::io_context& io_context, unsigned short port)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)), socket_(io_context) {}

void CServer::Start() {
    acceptor_.async_accept(socket_, [self = shared_from_this()](const boost::system::error_code& ec) {
        try {
            if (ec) {
                self->Start();
                return;
            }

            std::make_shared<HttpConnection>(std::move(self->socket_))->Start();

            self->Start();
        } catch (const std::exception& e) {
            // std::cerr << e.what() << '\n';
        }
    });
}

}  // namespace chat
}  // namespace pyc
