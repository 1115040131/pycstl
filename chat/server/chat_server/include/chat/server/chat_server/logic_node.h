#pragma once

#include <memory>

#include "chat/server/chat_server/csession.h"
#include "chat/server/chat_server/msg_node.h"

namespace pyc {

namespace chat {
class LogicNode {
    friend class LogicSystem;

public:
    LogicNode(const std::shared_ptr<CSession>& session, std::unique_ptr<RecvNode> recv_node)
        : session_(session), recv_node_(std::move(recv_node)) {}

private:
    std::shared_ptr<CSession> session_;
    std::unique_ptr<RecvNode> recv_node_;
};

}  // namespace chat
}  // namespace pyc
