#include "chat/server/status_server/status_service_impl.h"

#include <algorithm>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "chat/server/common/config_mgr.h"
#include "chat/server/status_server/define.h"

namespace pyc {
namespace chat {

static std::string generateUniqueString() {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    return boost::uuids::to_string(uuid);
};

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
        response->set_error(static_cast<int>(ErrorCode::kSuccess));
        response->set_host(server->host);
        response->set_port(server->port);
        response->set_token(generateUniqueString());
        {
            // TODO: 用 redis
            std::lock_guard<std::mutex> lock(mtx_);
            tokens_[request->uid()] = response->token();
        }
    }
    return grpc::Status::OK;
}

}  // namespace chat
}  // namespace pyc