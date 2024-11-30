#include "chat/server/chat_server/chat_service_impl.h"

#include <nlohmann/json.hpp>

#include "chat/common/error_code.h"
#include "chat/server/chat_server/user_mgr.h"

namespace pyc {
namespace chat {

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

grpc::Status ChatServiceImpl::NotifyTextChatMsg(grpc::ServerContext*, const TextChatMsgReq* request,
                                                TextChatMsgRsp* response) {
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
    notify["to_uid"] = request->to_uid();

    nlohmann::json text_array = nlohmann::json::array();
    for (const auto& msg : request->text_msgs()) {
        nlohmann::json text_json;
        text_json["msg_id"] = msg.msg_id();
        text_json["content"] = msg.msg_content();
        text_array.push_back(text_json);
    }
    notify["text_array"] = text_array;

    session->Send(notify.dump(), ReqId::kNotifyTextChatMsgReq);

    return grpc::Status::OK;
}

}  // namespace chat
}  // namespace pyc