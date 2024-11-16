#include "chat/server/chat_server/logic_system.h"

#include <nlohmann/json.hpp>

#include "chat/common/error_code.h"
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

void LogicSystem::LoginHandler(const std::shared_ptr<CSession>& session, const std::string& msg_data) {
    PYC_LOG_DEBUG("session: {}, msg_data: {}", session->GetUuid(), msg_data);

    nlohmann::json src_root = nlohmann::json::parse(msg_data, nullptr, false);
    nlohmann::json root;
    Defer defer([this, &session, &root]() { session->Send(root.dump(), ReqId::kChatLoginRes); });

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
        root["error"] = static_cast<int>(ErrorCode::kUidInvalid);
        return;
    }
    if (token_value != token) {
        root["error"] = static_cast<int>(ErrorCode::kTokenInvalid);
        return;
    }

    // 查询用户基本信息
    auto base_info = GetBaseInfo(uid);
    if (!base_info) {
        root["error"] = static_cast<int>(ErrorCode::kUidInvalid);
        return;
    }

    root["error"] = static_cast<int>(ErrorCode::kSuccess);
    root["token"] = token;
    root["base_info"] = base_info.value();

    // 获取申请列表

    // 获取好友列表

    auto server_name = GET_SECTION();
    RedisMgr::GetInstance().HIncrBy(kLoginCount, server_name, 1);
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

}  // namespace chat
}  // namespace pyc