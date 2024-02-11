#include "network/logic_system.h"

#include <fmt/core.h>
#include <nlohmann/json.hpp>

#include "network/utils.h"

namespace network {

void LogicSystem::PostMsgToQueue(std::unique_ptr<LogicNode> msg) {
    std::unique_lock<std::mutex> lock(mutex_);
    msg_queue_.push(std::move(msg));

    if (msg_queue_.size() == 1) {
        consume_.notify_one();
    }
}

LogicSystem::LogicSystem() {
    RegisterCallBack();
    work_thread_ = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem() {
    is_stop_ = true;
    consume_.notify_one();
    work_thread_.join();
}

void LogicSystem::RegisterCallBack() {
    callback_map_[MsgId::kMsgHelloWorld] = [this](const std::shared_ptr<Session>& session,
                                                  const std::string& msg_data) {
        HelloWorldCallBack(session, msg_data);
    };
}

void LogicSystem::HelloWorldCallBack(const std::shared_ptr<Session>& session, const std::string& msg_data) {
    auto reader = nlohmann::json::parse(msg_data);
    auto msg_id = reader["id"].get<MsgId>();

    // 打印消息
    auto receive_msg =
        fmt::format("Server receive msg id = {}, data = \"{}\"", msg_id, reader["data"].get<std::string>());
    fmt::println("[{}]: {}", __func__, receive_msg);
    reader["data"] = receive_msg;

    session->Send(reader.dump(), msg_id);
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
    auto msg_id = msg_node->recv_node_->msg_id_;
    fmt::println("[{}] recv msg id = {}", __func__, msg_id);
    auto iter = callback_map_.find(msg_id);
    if (iter != callback_map_.end()) {
        iter->second(msg_node->session_, std::string(msg_node->recv_node_->Data(), msg_node->recv_node_->Size()));
    }
    msg_queue_.pop();
}

}  // namespace network
