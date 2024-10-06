#include "chat/server/gate_server/cserver.h"

#include "chat/server/common/io_service_pool.h"
#include "chat/server/gate_server/http_connection.h"

namespace pyc {
namespace chat {

CServer::CServer(asio::io_context& io_context, unsigned short port)
    : io_context_(io_context), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {}

void CServer::Start() {
    auto& io_context = IOServicePool::GetInstance().GetIOService();
    // 创建新连接
    auto connection = std::make_shared<HttpConnection>(io_context);
    acceptor_.async_accept(connection->GetSocket(),
                           [self = shared_from_this(), connection](const boost::system::error_code& ec) {
                               try {
                                   if (ec) {
                                       self->Start();
                                       return;
                                   }

                                   // 启动新连接
                                   connection->Start();

                                   // 继续监听
                                   self->Start();
                               } catch (const std::exception& e) {
                                   PYC_LOG_ERROR("{}", e.what());
                               }
                           });
}

}  // namespace chat
}  // namespace pyc
