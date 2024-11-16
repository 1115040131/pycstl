#pragma once

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <unordered_map>

#include "chat/server/chat_server/csession.h"
#include "chat/server/chat_server/logic_node.h"
#include "chat/server/common/data.h"
#include "common/singleton.h"

namespace pyc {
namespace chat {

class LogicSystem : public Singleton<LogicSystem> {
    friend class Singleton<LogicSystem>;

    using FunCallBack = std::function<void(const std::shared_ptr<CSession>&, const std::string& msg_data)>;

private:
    LogicSystem();

public:
    ~LogicSystem();

    void PostMsgToQueue(std::unique_ptr<LogicNode> msg);

private:
    // 注册回调函数
    void RegisterCallBack();

    // 线程调用处理消息
    void DealMsg();

    // 处理队列中的第一个消息
    void DealFirstMsg();

    // 处理登录逻辑
    void LoginHandler(const std::shared_ptr<CSession>& session, const std::string& msg_data);

    // 获取用户基本信息
    std::optional<UserInfo> GetBaseInfo(int uid);

private:
    std::queue<std::unique_ptr<LogicNode>> msg_queue_{};
    std::mutex mutex_{};
    std::condition_variable consume_{};
    std::thread work_thread_{};
    bool is_stop_{false};
    std::unordered_map<ReqId, FunCallBack> callback_map_{};
};

}  // namespace chat
}  // namespace pyc
