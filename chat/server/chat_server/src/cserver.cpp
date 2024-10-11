#include "chat/server/chat_server/cserver.h"

#include "chat/server/chat_server/csession.h"
#include "chat/server/common/io_service_pool.h"
#include "chat/server/common/status_grpc_client.h"

namespace pyc {
namespace chat {

CServer::CServer(boost::asio::io_context& io_context, const std::string& name, unsigned short port)
    : io_context_(io_context), name_(name), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    StatusGrpcClient::GetInstance();  // 初始化 grpc 客户端
    StartAccept();

    PYC_LOG_INFO("{} listening on port {}", name_, port);
}

CServer::~CServer() { PYC_LOG_INFO("{} destruct", name_); }

void CServer::ClearSession(const std::string& uuid) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(uuid);

    printSessions();  // debug
}

void CServer::StartAccept() {
    auto& io_context = IOServicePool::GetInstance().GetIOService();
    auto session = std::make_shared<CSession>(io_context, this);
    acceptor_.async_accept(session->GetSocket(), [this, session](const boost::system::error_code& ec) {
        try {
            if (!ec) {
                session->Start();
                std::lock_guard<std::mutex> lock(mutex_);
                sessions_.emplace(session->GetUuid(), session);

                printSessions();  // debug
            } else {
                PYC_LOG_ERROR("{}", ec.message());
            }
            StartAccept();
        } catch (const std::exception& e) {
            PYC_LOG_ERROR("{}", e.what());
        }
    });
}

void CServer::printSessions() {
    PYC_LOG_DEBUG("========== Sessions ==========");
    for (const auto& [uuid, session] : sessions_) {
        PYC_LOG_DEBUG("uuid: {}", uuid);
    }
    PYC_LOG_DEBUG("==============================");
}

}  // namespace chat
}  // namespace pyc