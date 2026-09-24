module;

#include <cstddef>
#include <exception>
#include <memory>
#include <mutex>
#include <string>

#include <boost/asio.hpp>

#include "logger/logger.h"

module chat.server.chat_server;

import :csession;
import :user_mgr;
import chat.server.chat_server.chat_grpc_client;
import chat.server.common.io_service_pool;
import chat.server.common.status_grpc_client;

namespace pyc {
namespace chat {

CServer::CServer(boost::asio::io_context& io_context, const std::string& name, unsigned short port)
    : name_(name), acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    // 初始化 grpc 客户端
    StatusGrpcClient::GetInstance();
    ChatGrpcClient::GetInstance();

    StartAccept();

    LogInfo("{} listening on port {}", name_, port);
}

CServer::~CServer() { LogInfo("{} destruct", name_); }

void CServer::ClearSession(const std::string& session_id) {
    auto iter = sessions_.find(session_id);
    if (iter != sessions_.end()) {
        UserMgr::GetInstance().RemoveUserSeesion(iter->second->GetUserId());
    }

    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(session_id);

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
                sessions_.emplace(session->GetSessionId(), session);

                printSessions();  // debug
            } else {
                LogError("{}", ec.message());
            }
            StartAccept();
        } catch (const std::exception& e) {
            LogError("{}", e.what());
        }
    });
}

void CServer::printSessions() {
    LogDebug("========== Sessions ==========");
    for (const auto& [uuid, session] : sessions_) {
        LogDebug("uuid: {}", uuid);
    }
    LogDebug("==============================");
}

}  // namespace chat
}  // namespace pyc