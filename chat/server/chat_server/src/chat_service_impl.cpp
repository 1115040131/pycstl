#include "chat/server/chat_server/chat_service_impl.h"

#include <nlohmann/json.hpp>

#include "chat/common/error_code.h"
#include "chat/server/chat_server/user_mgr.h"

namespace pyc {
namespace chat {

// TODO: remove
template <typename... Targs>
void DUMMY_CODE(Targs&&... /* unused */) {}

ChatServiceImpl::ChatServiceImpl() {}

grpc::Status ChatServiceImpl::NotifyAddFriend(grpc::ServerContext*, const AddFriendReq* request,
                                              AddFriendRsp* response) {
    // 查找用户是否在本服务器
    auto to_uid = request->to_uid();
    auto session = UserMgr::GetInstance().GetSession(to_uid);
    if (!session) {
        response->set_error(static_cast<int>(ErrorCode::kRpcFailed));
        return grpc::Status::OK;
    }

    nlohmann::json notify;
    notify["error"] = ErrorCode::kSuccess;
    notify["apply_uid"] = request->apply_uid();
    notify["apply_name"] = request->apply_name();
    notify["nick"] = request->nick();
    notify["desc"] = request->desc();
    notify["sex"] = request->sex();
    notify["icon"] = request->icon();

    session->Send(notify.dump(), ReqId::kNotifyAddFriendReq);

    return grpc::Status::OK;
}

grpc::Status ChatServiceImpl::NotifyAuthFriend(grpc::ServerContext*, const AuthFriendReq* request,
                                               AuthFriendRsp* response) {
    // 查找用户是否在本服务器
    auto to_uid = request->to_uid();
    auto session = UserMgr::GetInstance().GetSession(to_uid);
    if (!session) {
        response->set_error(static_cast<int>(ErrorCode::kRpcFailed));
        return grpc::Status::OK;
    }

    nlohmann::json notify;
    notify["error"] = ErrorCode::kSuccess;
    notify["from_uid"] = request->from_uid();
    notify["name"] = request->name();
    notify["nick"] = request->nick();
    notify["sex"] = request->sex();
    notify["icon"] = request->icon();

    session->Send(notify.dump(), ReqId::kNotifyAuthFriendReq);

    return grpc::Status::OK;
}

grpc::Status ChatServiceImpl::NotifyTextChatMsg(grpc::ServerContext* context, const TextChatMsgReq* request,
                                                TextChatMsgRsp* response) {
    DUMMY_CODE(context, request, response);

    return grpc::Status::OK;
}

}  // namespace chat
}  // namespace pyc