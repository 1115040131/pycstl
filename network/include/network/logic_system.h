#pragma once

#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <unordered_map>

#include "common/singleton.h"
#include "network/logic_node.h"
#include "network/msg_node.h"
#include "network/session.h"

namespace network {

class LogicSystem : public pyc::Singleton<LogicSystem> {
    friend class pyc::Singleton<LogicSystem>;
    using FunCallBack = std::function<void(const std::shared_ptr<Session>&, const std::string& msg_data)>;

public:
    void PostMsgToQueue(std::unique_ptr<LogicNode> msg);

private:
    LogicSystem();

    ~LogicSystem();

    /// @brief 注册回调函数
    void RegisterCallBack();

    /// @brief 处理 kMsgHelloWorld 类型的消息
    void HelloWorldCallBack(const std::shared_ptr<Session>& session, const std::string& msg_data);

    /// @brief 线程调用处理消息
    void DealMsg();

    /// @brief 处理队列中的第一个消息
    void DealFirstMsg();

private:
    std::queue<std::unique_ptr<LogicNode>> msg_queue_;
    std::mutex mutex_;
    std::condition_variable consume_;
    std::thread work_thread_;
    bool is_stop_{false};
    std::unordered_map<MsgId, FunCallBack> callback_map_;
};

}  // namespace network
