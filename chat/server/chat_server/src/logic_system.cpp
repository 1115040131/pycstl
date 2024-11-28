#include "chat/server/chat_server/logic_system.h"

#include <ranges>

#include <nlohmann/json.hpp>

#include "chat/common/error_code.h"
#include "chat/server/chat_server/chat_grpc_client.h"
#include "chat/server/chat_server/user_mgr.h"
#include "chat/server/common/config_mgr.h"
#include "chat/server/common/defer.h"
#include "chat/server/common/mysql_mgr.h"
#include "chat/server/common/redis_mgr.h"

namespace pyc {
namespace chat {

LogicSystem::LogicSystem() {
    RegisterCallBack();
    work_thread_ = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem() {
    is_stop_ = true;
    consume_.notify_all();
    work_thread_.join();
    PYC_LOG_INFO("LogicSystem Exit.");
}

void LogicSystem::PostMsgToQueue(std::unique_ptr<LogicNode> msg) {
    std::unique_lock<std::mutex> lock(mutex_);
    msg_queue_.push(std::move(msg));

    if (msg_queue_.size() == 1) {
        consume_.notify_one();
    }
}

void LogicSystem::RegisterCallBack() {
    callback_map_[ReqId::kChatLogin] = [this](const std::shared_ptr<CSession>& session,
                                              const std::string& msg_data) { LoginHandler(session, msg_data); };
    callback_map_[ReqId::kSearchUserReq] = [this](const std::shared_ptr<CSession>& session,
                                                  const std::string& msg_data) {
        SearchInfoHandler(session, msg_data);
    };
    callback_map_[ReqId::kAddFriendReq] = [this](const std::shared_ptr<CSession>& session,
                                                 const std::string& msg_data) {
        AddFriendHandler(session, msg_data);
    };
    callback_map_[ReqId::kAuthFriendReq] = [this](const std::shared_ptr<CSession>& session,
                                                  const std::string& msg_data) {
        AuthFriendHandler(session, msg_data);
    };
}

void LogicSystem::DealMsg() {
    for (;;) {
        std::unique_lock<std::mutex> lock(mutex_);

        // 队列为空用条件变量等待
        while (msg_queue_.empty() && !is_stop_) {
            consume_.wait(lock);
        }

        // 关闭状态, 及时处理清空队列
        if (is_stop_) {
            while (!msg_queue_.empty()) {
                DealFirstMsg();
            }

            break;
        }

        // 正常处理一条消息
        DealFirstMsg();
    }
}

void LogicSystem::DealFirstMsg() {
    auto& msg_node = msg_queue_.front();
    auto msg_id = msg_node->recv_node_->GetReqId();
    PYC_LOG_DEBUG("recv msg id: {}", ToString(msg_id));
    auto iter = callback_map_.find(msg_id);
    if (iter != callback_map_.end()) {
        iter->second(msg_node->session_, std::string(msg_node->recv_node_->Data(), msg_node->recv_node_->Size()));
    }
    msg_queue_.pop();
}

std::optional<UserInfo> LogicSystem::GetBaseInfo(int uid) {
    auto key = fmt::format("{}{}", kUserBaseInfo, uid);

    // 优先 redis 中查询
    auto info_str = RedisMgr::GetInstance().Get(key);
    if (info_str) {
        auto j = nlohmann::json::parse(info_str.value(), nullptr, false);
        if (j.is_discarded()) {
            PYC_LOG_ERROR("Failed to parse Json data: {}", info_str.value());
            return std::nullopt;
        }

        return j.get<UserInfo>();
    }

    // redis 中没有则查询 mysql
    auto user_info = MysqlMgr::GetInstance().GetUser(uid);
    if (!user_info) {
        return std::nullopt;
    }

    // 将数据写入 redis 缓存中
    nlohmann::json j = user_info.value();
    auto result = RedisMgr::GetInstance().Set(key, j.dump());
    if (!result) {
        PYC_LOG_ERROR("Redis set {} {} error", key, j.dump());
    }

    return user_info.value();
}

std::optional<UserInfo> LogicSystem::GetBaseInfo(const std::string& name) {
    auto key = fmt::format("{}{}", kNameInfo, name);

    // 优先 redis 中查询
    auto info_str = RedisMgr::GetInstance().Get(key);
    if (info_str) {
        auto j = nlohmann::json::parse(info_str.value(), nullptr, false);
        if (j.is_discarded()) {
            PYC_LOG_ERROR("Failed to parse Json data: {}", info_str.value());
            return std::nullopt;
        }

        return j.get<UserInfo>();
    }

    // redis 中没有则查询 mysql
    auto user_info = MysqlMgr::GetInstance().GetUser(name);
    if (!user_info) {
        return std::nullopt;
    }

    // 将数据写入 redis 缓存中
    nlohmann::json j = user_info.value();
    auto result = RedisMgr::GetInstance().Set(key, j.dump());
    if (!result) {
        PYC_LOG_ERROR("Redis set {} {} error", key, j.dump());
    }

    return user_info.value();
}

void LogicSystem::LoginHandler(const std::shared_ptr<CSession>& session, const std::string& msg_data) {
    PYC_LOG_DEBUG("session: {}, msg_data: {}", session->GetSessionId(), msg_data);

    nlohmann::json src_root = nlohmann::json::parse(msg_data, nullptr, false);
    nlohmann::json root;
    Defer defer([&session, &root]() { session->Send(root.dump(), ReqId::kChatLoginRes); });

    if (src_root.is_discarded()) {
        PYC_LOG_ERROR("Failed to parse Json data");
        root["error"] = ErrorCode::kJsonError;
        return;
    }

    int uid = src_root.value("uid", 0);
    auto token = src_root.value("token", "");

    // 从 redis 中查询 token 是否正确
    auto token_key = fmt::format("{}{}", kUserTokenPrefix, uid);
    auto token_value = RedisMgr::GetInstance().Get(token_key);
    if (!token_value) {
        root["error"] = ErrorCode::kUidInvalid;
        return;
    }
    if (token_value != token) {
        root["error"] = ErrorCode::kTokenInvalid;
        return;
    }

    // 查询用户基本信息
    auto base_info = GetBaseInfo(uid);
    if (!base_info) {
        root["error"] = ErrorCode::kUidInvalid;
        return;
    }

    root["error"] = ErrorCode::kSuccess;
    root["token"] = token;
    root["base_info"] = base_info.value();

    // 获取申请列表
    auto apply_list = MysqlMgr::GetInstance().GetApplyList(uid, 0);
    if (apply_list) {
        nlohmann::json apply_list_json;
        for (const auto& apply : apply_list.value()) {
            apply_list_json.push_back(apply);
        }
        root["apply_list"] = apply_list_json;
    }

    // 获取好友列表
    auto friend_list = MysqlMgr::GetInstance().GetFriendList(uid);
    if (friend_list) {
        nlohmann::json friend_list_json;
        for (const auto& friend_info : friend_list.value()) {
            friend_list_json.push_back(friend_info);
        }
        root["friend_list"] = friend_list_json;
    }

    // 增加登录数量
    auto server_name = GET_SECTION();
    RedisMgr::GetInstance().HIncrBy(kLoginCount, server_name, 1);

    // session 绑定用户 id
    session->SetUserId(base_info->uid);

    // 为用户设置登录 ip server 的名字
    RedisMgr::GetInstance().Set(fmt::format("{}{}", kUserIpPrefix, uid), server_name);

    // uid 和 session 绑定, 方便以后踢人操作
    UserMgr::GetInstance().SetUserSession(uid, session);
}

void LogicSystem::SearchInfoHandler(const std::shared_ptr<CSession>& session, const std::string& msg_data) {
    PYC_LOG_DEBUG("session: {}, msg_data: {}", session->GetSessionId(), msg_data);

    nlohmann::json src_root = nlohmann::json::parse(msg_data, nullptr, false);
    nlohmann::json root;
    Defer defer([&session, &root]() { session->Send(root.dump(), ReqId::kSearchUserRes); });

    if (src_root.is_discarded()) {
        PYC_LOG_ERROR("Failed to parse Json data");
        root["error"] = ErrorCode::kJsonError;
        return;
    }

    auto uid_str = src_root.value("uid", "");
    if (uid_str == "") {
        PYC_LOG_ERROR("uid is empty");
        root["error"] = ErrorCode::kUidInvalid;
        return;
    }

    std::optional<UserInfo> search_info;

    // 优先按照 id 找
    if (std::ranges::find_if(uid_str, [](char c) { return !std::isdigit(c); }) == uid_str.end()) {
        search_info = GetBaseInfo(std::stoi(uid_str));
    }

    // 再按照 name 找
    if (!search_info) {
        search_info = GetBaseInfo(uid_str);
    }

    if (!search_info) {
        PYC_LOG_ERROR("Failed to find user info {}", uid_str);
        root["error"] = ErrorCode::kUidInvalid;
        return;
    }

    root["error"] = ErrorCode::kSuccess;
    root["search_info"] = search_info.value();
}

void LogicSystem::AddFriendHandler(const std::shared_ptr<CSession>& session, const std::string& msg_data) {
    PYC_LOG_DEBUG("session: {}, msg_data: {}", session->GetSessionId(), msg_data);

    nlohmann::json request = nlohmann::json::parse(msg_data, nullptr, false);
    nlohmann::json response;
    Defer defer([&session, &response]() { session->Send(response.dump(), ReqId::kAddFriendRes); });

    if (request.is_discarded()) {
        PYC_LOG_ERROR("Failed to parse Json data");
        response["error"] = ErrorCode::kJsonError;
        return;
    }

    int uid = request.value("uid", 0);
    int to_uid = request.value("to_uid", 0);
    auto apply_name = request.value("apply_name", "");
    auto back_name = request.value("back_name", "");

    // 查询自己的基本信息
    auto base_info = GetBaseInfo(uid);
    if (!base_info) {
        response["error"] = ErrorCode::kUidInvalid;
        return;
    }

    // 更新数据库
    if (!(MysqlMgr::GetInstance().AddFriendAppply(uid, to_uid) == true)) {
        PYC_LOG_ERROR("Failed to update database");
        response["error"] = ErrorCode::kNetworkError;
        return;
    }

    // 数据库操作结束认为已经成功, 后续为通知对端
    response["error"] = ErrorCode::kSuccess;

    // 查询对方服务器地址
    auto to_server_name = RedisMgr::GetInstance().Get(fmt::format("{}{}", kUserIpPrefix, to_uid));
    if (!to_server_name) {
        PYC_LOG_DEBUG("User {} not online", to_uid);
        return;
    }
    // 对方在同一个服务器上
    if (to_server_name == GET_SECTION()) {
        auto session = UserMgr::GetInstance().GetSession(to_uid);
        if (!session) {
            PYC_LOG_WARN("User {} not online", to_uid);
        } else {
            nlohmann::json notify;
            notify["error"] = ErrorCode::kSuccess;
            notify["apply_uid"] = uid;
            notify["apply_name"] = apply_name;
            notify["nick"] = base_info->nick;
            notify["desc"] = base_info->desc;
            notify["sex"] = base_info->sex;
            notify["icon"] = base_info->icon;
            session->Send(notify.dump(), ReqId::kNotifyAddFriendReq);
        }
    } else {
        // 对方在其他服务器上
        AddFriendReq rpc_request;
        rpc_request.set_apply_uid(uid);
        rpc_request.set_apply_name(apply_name);
        rpc_request.set_nick(base_info->nick);
        rpc_request.set_desc(base_info->desc);
        rpc_request.set_sex(base_info->sex);
        rpc_request.set_icon(base_info->icon);
        rpc_request.set_to_uid(to_uid);
        ChatGrpcClient::GetInstance().NotifyAddFriend(to_server_name.value(), rpc_request);
    }
}

void LogicSystem::AuthFriendHandler(const std::shared_ptr<CSession>& session, const std::string& msg_data) {
    PYC_LOG_DEBUG("session: {}, msg_data: {}", session->GetSessionId(), msg_data);

    nlohmann::json request = nlohmann::json::parse(msg_data, nullptr, false);
    nlohmann::json response;
    Defer defer([&session, &response]() { session->Send(response.dump(), ReqId::kAuthFriendRes); });

    if (request.is_discarded()) {
        PYC_LOG_ERROR("Failed to parse Json data");
        response["error"] = ErrorCode::kJsonError;
        return;
    }

    int from_uid = request.value("from_uid", 0);
    int to_uid = request.value("to_uid", 0);
    auto back_name = request.value("back_name", "");

    PYC_LOG_INFO("{} to {}", from_uid, to_uid);

    // 查询基本信息
    auto from_base_info = GetBaseInfo(from_uid);
    auto to_base_info = GetBaseInfo(to_uid);
    if (!from_base_info || !to_base_info) {
        response["error"] = ErrorCode::kUidInvalid;
        return;
    }

    // 更新数据库
    if (!(MysqlMgr::GetInstance().AuthFriendApply(from_uid, to_uid) == true) ||
        !(MysqlMgr::GetInstance().AddFriend(from_uid, to_uid, back_name) == true)) {
        PYC_LOG_ERROR("Failed to update database");
        response["error"] = ErrorCode::kNetworkError;
        return;
    }

    // 数据库操作结束认为已经成功, 后续为通知对端
    response["error"] = ErrorCode::kSuccess;
    response["to_uid"] = to_uid;
    response["name"] = to_base_info->name;
    response["nick"] = to_base_info->nick;
    response["sex"] = to_base_info->sex;
    response["icon"] = to_base_info->icon;

    // 查询对方服务器地址
    auto to_server_name = RedisMgr::GetInstance().Get(fmt::format("{}{}", kUserIpPrefix, to_uid));
    if (!to_server_name) {
        PYC_LOG_DEBUG("User {} not online", to_uid);
        return;
    }

    // 对方在同一个服务器上
    if (to_server_name == GET_SECTION()) {
        auto session = UserMgr::GetInstance().GetSession(to_uid);
        if (!session) {
            PYC_LOG_WARN("User {} not online", to_uid);
        } else {
            nlohmann::json notify;
            notify["error"] = ErrorCode::kSuccess;
            notify["from_uid"] = from_uid;
            notify["name"] = from_base_info->name;
            notify["nick"] = from_base_info->nick;
            notify["sex"] = from_base_info->sex;
            notify["icon"] = from_base_info->icon;
            session->Send(notify.dump(), ReqId::kNotifyAuthFriendReq);
        }
    } else {
        // 对方在其他服务器上
        AuthFriendReq rpc_request;
        rpc_request.set_from_uid(from_uid);
        rpc_request.set_to_uid(to_uid);
        rpc_request.set_name(from_base_info->name);
        rpc_request.set_nick(from_base_info->nick);
        rpc_request.set_sex(from_base_info->sex);
        rpc_request.set_icon(from_base_info->icon);
        ChatGrpcClient::GetInstance().NotifyAuthFriend(to_server_name.value(), rpc_request);
    }
}

}  // namespace chat
}  // namespace pyc