#include "chat/server/chat_server/chat_grpc_client.h"

#include <grpcpp/create_channel.h>

#include "chat/common/error_code.h"
#include "chat/server/chat_server/define.h"
#include "chat/server/common/config_mgr.h"
#include "chat/server/common/defer.h"
#include "common/connection_pool.h"

namespace pyc {
namespace chat {

// TODO: remove
template <typename... Targs>
void DUMMY_CODE(Targs&&... /* unused */) {}

class ChatConnectionPool : public ConnectionPool<std::unique_ptr<ChatService::Stub>> {
public:
    ChatConnectionPool(std::string_view host, std::string_view port, size_t size) {
        auto target = fmt::format("{}:{}", host, port);
        for (std::size_t i = 0; i < size; ++i) {
            connections_.push(
                ChatService::NewStub(grpc::CreateChannel(target, grpc::InsecureChannelCredentials())));
        }
    }
};

ChatGrpcClient::ChatGrpcClient() {
    fmt::println("========== ChatGrpcClient setup ==========");

    GET_SECTION_CONFIG(peer_servers, "PeerServers");

    std::unordered_set<std::string> servers;

    {
        std::stringstream ss(peer_servers);
        std::string server;

        while (std::getline(ss, server, ',')) {
            if (servers.count(server) > 0) {
                PYC_LOG_ERROR("Same peer server {}!", server);
            } else {
                servers.insert(server);
            }
        }
    }

    if (servers.empty()) {
        PYC_LOG_WARN("No peer server found!");
    }
    for (const auto& server : servers) {
        GET_CONFIG(host, server, "Host");
        GET_CONFIG(port, server, "RpcPort");
        pools_[server] = std::make_unique<ChatConnectionPool>(host, port, 5);
        PYC_LOG_INFO("Peer Server {} at {}:{} connect", server, host, port);
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
        PYC_LOG_ERROR("Server {} not found", server_name);
        response.set_error(static_cast<int>(ErrorCode::kRpcFailed));
        return response;
    }

    auto& pool = iter->second;
    auto connection = pool->GetConnection();
    if (!connection) {
        PYC_LOG_ERROR("Get connection failed");
        response.set_error(static_cast<int>(ErrorCode::kRpcFailed));
        return response;
    }
    Defer defer([this, &pool, &connection]() { pool->ReturnConnection(std::move(*connection)); });

    grpc::ClientContext context;
    auto status = connection.value()->NotifyAddFriend(&context, request, &response);

    if (!status.ok()) {
        PYC_LOG_ERROR("Rpc failed: {}", status.error_message());
        response.set_error(static_cast<int>(ErrorCode::kRpcFailed));
    }

    return response;
}

AuthFriendRsp ChatGrpcClient::NotifyAuthFriend(const std::string& server_name, const AuthFriendReq& request) {
    DUMMY_CODE(server_name, request);
    return {};
}

TextChatMsgRsp ChatGrpcClient::NotifyTextChatMsg(const std::string& server_name, const TextChatMsgReq& request,
                                                 const nlohmann::json& return_value) {
    DUMMY_CODE(server_name, request, return_value);
    return {};
}

}  // namespace chat
}  // namespace pyc
