module;

#include <memory>

export module network.logic:node;

export import network.base;
export import network.msg_node;

export namespace network {

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
