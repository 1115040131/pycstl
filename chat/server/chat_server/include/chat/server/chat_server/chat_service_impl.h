#pragma once

#include "chat/server/proto/chat.grpc.pb.h"

namespace pyc {
namespace chat {

class ChatServiceImpl final : public ChatService::Service {
public:
    ChatServiceImpl();

    virtual grpc::Status NotifyAddFriend(grpc::ServerContext* context, const AddFriendReq* request,
                                         AddFriendRsp* response) override;

    virtual grpc::Status NotifyAuthFriend(grpc::ServerContext* context, const AuthFriendReq* request,
                                          AuthFriendRsp* response) override;

    virtual grpc::Status NotifyTextChatMsg(grpc::ServerContext* context, const TextChatMsgReq* request,
                                           TextChatMsgRsp* response) override;
};

}  // namespace chat
}  // namespace pyc
