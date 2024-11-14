#include "chat/server/status_server/status_service_impl.h"

#include <algorithm>

#include "chat/server/common/config_mgr.h"
#include "chat/server/common/redis_mgr.h"
#include "chat/server/common/utils.h"
#include "chat/server/status_server/define.h"

namespace pyc {
namespace chat {

StatusServiceImpl::StatusServiceImpl() {
    for (const auto& server_name : std::vector{"ChatServer1", "ChatServer2"}) {
        GET_CONFIG(name, server_name, "Name");
        GET_CONFIG(host, server_name, "Host");
        GET_CONFIG(port, server_name, "Port");

        ChatServer server{
            .name = name,
            .host = host,
            .port = port,
        };

        servers_[name] = server;
    }
}

std::optional<ChatServer> StatusServiceImpl::selectChatServer() {
    std::lock_guard<std::mutex> lock(mtx_);

    for (auto& [_, server] : servers_) {
        auto count_str = RedisMgr::GetInstance().HGet(kLoginCount, server.name);
        if (!count_str) {
            PYC_LOG_WARN("Get [{}] login count fail", server.name);
            server.connection_count = std::numeric_limits<int>::max();
        } else {
            server.connection_count = std::stoi(*count_str);
        }
    }

    auto iter = std::ranges::min_element(servers_, [](const auto& lhs, const auto& rhs) {
        return lhs.second.connection_count < rhs.second.connection_count;
    });
    if (iter == servers_.end()) {
        return std::nullopt;
    }
    return iter->second;
}

grpc::Status StatusServiceImpl::GetChatServer(grpc::ServerContext*, const pyc::chat::GetChatServerReq* request,
                                              pyc::chat::GetChatServerRsp* response) {
    auto server = selectChatServer();
    if (!server) {
        response->set_error(static_cast<int>(ErrorCode::kNetworkError));
    } else {
        auto token = generateUniqueString();
        auto token_key = fmt::format("{}{}", kUserTokenPrefix, request->uid());
        auto result = RedisMgr::GetInstance().Set(token_key, token);
        if (!result) {
            response->set_error(static_cast<int>(ErrorCode::kNetworkError));
        }

        response->set_error(static_cast<int>(ErrorCode::kSuccess));
        response->set_host(server->host);
        response->set_port(server->port);
        response->set_token(token);
    }
    return grpc::Status::OK;
}

grpc::Status StatusServiceImpl::Login(grpc::ServerContext*, const pyc::chat::LoginReq* request,
                                      pyc::chat::LoginRsp* response) {
    auto uid = request->uid();
    auto token = request->token();

    std::lock_guard<std::mutex> lock(mtx_);

    auto token_key = fmt::format("{}{}", kUserTokenPrefix, uid);
    auto token_value = RedisMgr::GetInstance().Get(token_key);
    if (!token_value) {
        response->set_error(static_cast<int>(ErrorCode::kUidInvalid));
        return grpc::Status::OK;
    }

    if (token_value != token) {
        response->set_error(static_cast<int>(ErrorCode::kTokenInvalid));
        return grpc::Status::OK;
    }
    response->set_error(static_cast<int>(ErrorCode::kSuccess));
    response->set_uid(uid);
    response->set_token(token);

    return grpc::Status::OK;
}

}  // namespace chat
}  // namespace pyc