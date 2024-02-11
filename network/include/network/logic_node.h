#pragma once

#include <memory>

#include "network/msg_node.h"
#include "network/session.h"

namespace network {

class LogicNode {
    friend class LogicSystem;

public:
    LogicNode(const std::shared_ptr<Session>& session, std::unique_ptr<RecvNode> recv_node)
        : session_(session), recv_node_(std::move(recv_node)) {}

private:
    std::shared_ptr<Session> session_;
    std::unique_ptr<RecvNode> recv_node_;
};

}  // namespace network
