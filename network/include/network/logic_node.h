#pragma once

#include <memory>

#include "network/base/session.h"
#include "network/msg_node.h"

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
