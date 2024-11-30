#pragma once

#include <unordered_map>

#include <nlohmann/json.hpp>

#include "chat/server/proto/chat.grpc.pb.h"
#include "common/singleton.h"

namespace pyc {
namespace chat {

class ChatConnectionPool;

class ChatGrpcClient : public Singleton<ChatGrpcClient> {
    friend class Singleton<ChatGrpcClient>;

private:
    ChatGrpcClient();

public:
    ~ChatGrpcClient();

    AddFriendRsp NotifyAddFriend(const std::string& server_name, const AddFriendReq& request);

    AuthFriendRsp NotifyAuthFriend(const std::string& server_name, const AuthFriendReq& request);

    TextChatMsgRsp NotifyTextChatMsg(const std::string& server_name, const TextChatMsgReq& request);

private:
    std::unordered_map<std::string, std::unique_ptr<ChatConnectionPool>> pools_;
};

}  // namespace chat
}  // namespace pyc
