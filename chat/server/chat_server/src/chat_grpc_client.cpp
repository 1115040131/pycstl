module;

#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <fmt/base.h>
#include <grpcpp/create_channel.h>

#include "chat/common/error_code.h"
#include "chat/server/proto/chat.grpc.pb.h"
#include "common/connection_pool.h"
#include "common/utils.h"
#include "logger/logger.h"

module chat.server.chat_server.chat_grpc_client;

import chat.server.chat_server.define;
import chat.server.common.config_mgr;
import chat.server.common.defer;

namespace pyc {
namespace chat {

class ChatConnectionPool : public ConnectionPool<std::unique_ptr<ChatService::Stub>> {
public:
    ChatConnectionPool(std::string_view host, std::string_view port, size_t size) {
        auto target = JoinHostPort(host, port);
        for (std::size_t i = 0; i < size; ++i) {
            connections_.push(
                ChatService::NewStub(grpc::CreateChannel(target, grpc::InsecureChannelCredentials())));
        }
    }
};

ChatGrpcClient::ChatGrpcClient() {
    fmt::println("========== ChatGrpcClient setup ==========");

    auto peer_servers = GetSectionConfigOrDie("PeerServers");

    std::unordered_set<std::string> servers;

    {
        std::stringstream ss(peer_servers);
        std::string server;

        while (std::getline(ss, server, ',')) {
            if (servers.count(server) > 0) {
                LogError("Same peer server {}!", server);
            } else {
                servers.insert(server);
            }
        }
    }

    if (servers.empty()) {
        LogWarn("No peer server found!");
    }
    for (const auto& server : servers) {
        auto host = GetConfigOrDie(server, "Host");
        auto port = GetConfigOrDie(server, "RpcPort");
        pools_[server] = std::make_unique<ChatConnectionPool>(host, port, 5);
        LogInfo("Peer Server {} at {}:{} connect", server, host, port);
    }

    fmt::println("==========================================");
}

ChatGrpcClient::~ChatGrpcClient() {
    for (auto& pool : pools_) {
        pool.second->Close();
    }
}

AddFriendRsp ChatGrpcClient::NotifyAddFriend(const std::string& server_name, const AddFriendReq& request) {
    AddFriendRsp response;

    auto iter = pools_.find(server_name);
    if (iter == pools_.end()) {
        LogError("Server {} not found", server_name);
        response.set_error(static_cast<int>(ErrorCode::kRpcFailed));
        return response;
    }

    auto& pool = iter->second;
    auto connection = pool->GetConnection();
    if (!connection) {
        LogError("Get connection failed");
        response.set_error(static_cast<int>(ErrorCode::kRpcFailed));
        return response;
    }
    Defer defer([&pool, &connection]() { pool->ReturnConnection(std::move(*connection)); });

    grpc::ClientContext context;
    auto status = connection.value()->NotifyAddFriend(&context, request, &response);

    if (!status.ok()) {
        LogError("Rpc failed: {}", status.error_message());
        response.set_error(static_cast<int>(ErrorCode::kRpcFailed));
    }

    return response;
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(const std::string& server_name, const AuthFriendReq& request) {
    AuthFriendRsp response;

    auto iter = pools_.find(server_name);
    if (iter == pools_.end()) {
        LogError("Server {} not found", server_name);
        response.set_error(static_cast<int>(ErrorCode::kRpcFailed));
        return response;
    }

    auto& pool = iter->second;
    auto connection = pool->GetConnection();
    if (!connection) {
        LogError("Get connection failed");
        response.set_error(static_cast<int>(ErrorCode::kRpcFailed));
        return response;
    }
    Defer defer([&pool, &connection]() { pool->ReturnConnection(std::move(*connection)); });

    grpc::ClientContext context;
    auto status = connection.value()->NotifyAuthFriend(&context, request, &response);

    if (!status.ok()) {
        LogError("Rpc failed: {}", status.error_message());
        response.set_error(static_cast<int>(ErrorCode::kRpcFailed));
    }

    return response;
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(const std::string& server_name, const TextChatMsgReq& request) {
    TextChatMsgRsp response;

    auto iter = pools_.find(server_name);
    if (iter == pools_.end()) {
        LogError("Server {} not found", server_name);
        response.set_error(static_cast<int>(ErrorCode::kRpcFailed));
        return response;
    }

    auto& pool = iter->second;
    auto connection = pool->GetConnection();
    if (!connection) {
        LogError("Get connection failed");
        response.set_error(static_cast<int>(ErrorCode::kRpcFailed));
        return response;
    }
    Defer defer([&pool, &connection]() { pool->ReturnConnection(std::move(*connection)); });

    grpc::ClientContext context;
    auto status = connection.value()->NotifyTextChatMsg(&context, request, &response);

    if (!status.ok()) {
        LogError("Rpc failed: {}", status.error_message());
        response.set_error(static_cast<int>(ErrorCode::kRpcFailed));
    }

    return response;
}

}  // namespace chat
}  // namespace pyc
