#pragma once

#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include <grpcpp/grpcpp.h>

#include "chat/server/proto/status.grpc.pb.h"

namespace pyc {
namespace chat {

struct ChatServer {
    int connection_count = 0;
    std::string name;
    std::string host;
    std::string port;
};

class StatusServiceImpl final : public StatusService::Service {
public:
    StatusServiceImpl();

    virtual grpc::Status GetChatServer(grpc::ServerContext* context, const pyc::chat::GetChatServerReq* request,
                                       pyc::chat::GetChatServerRsp* response) override;

    virtual grpc::Status Login(grpc::ServerContext* context, const pyc::chat::LoginReq* request,
                               pyc::chat::LoginRsp* response) override;

private:
    std::optional<ChatServer> selectChatServer();

private:
    // int server_index_ = 0;
    std::unordered_map<std::string, ChatServer> servers_;
    std::unordered_map<int, std::string> tokens_;
    std::mutex mtx_;
};

}  // namespace chat
}  // namespace pyc
