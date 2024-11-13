#include "chat/server/chat_server/chat_service_impl.h"

namespace pyc {
namespace chat {

// TODO: remove
template <typename... Targs>
void DUMMY_CODE(Targs&&... /* unused */) {}

ChatServiceImpl::ChatServiceImpl() {}

grpc::Status ChatServiceImpl::NotifyAddFriend(grpc::ServerContext* context, const AddFriendReq* request,
                                              AddFriendRsp* response) {
    DUMMY_CODE(context, request, response);
    return grpc::Status::OK;
}

grpc::Status ChatServiceImpl::NotifyAuthFriend(grpc::ServerContext* context, const AuthFriendReq* request,
                                               AuthFriendRsp* response) {
    DUMMY_CODE(context, request, response);

    return grpc::Status::OK;
}

grpc::Status ChatServiceImpl::NotifyTextChatMsg(grpc::ServerContext* context, const TextChatMsgReq* request,
                                                TextChatMsgRsp* response) {
    DUMMY_CODE(context, request, response);

    return grpc::Status::OK;
}

}  // namespace chat
}  // namespace pyc