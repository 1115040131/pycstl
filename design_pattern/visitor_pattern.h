#pragma once

#include <memory>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

namespace pyc {

struct MessageVisitor;

struct Message {
    virtual void accept(MessageVisitor* visitor) = 0;
    virtual ~Message() = default;
};

template <typename Derived>
struct MessageImpl : Message {
    void accept(MessageVisitor* visitor) override {
        static_assert(std::is_base_of_v<MessageImpl, Derived>, "Derived must be derived from MessageImpl");
        visitor->visit(static_cast<Derived*>(this));
    }
};

struct MoveMessage : MessageImpl<MoveMessage> {
    glm::vec3 velocityChange;

    // void accept(MessageVisitor* visitor) override { visitor->visit(this); }
};

struct JumpMessage : MessageImpl<JumpMessage> {
    double jumpHeight;

    // void accept(MessageVisitor* visitor) override { visitor->visit(this); }
};

struct MessageVisitor {
    virtual void visit(MoveMessage* msg) {}
    virtual void visit(JumpMessage* msg) {}
};

struct GameObject;

struct Component {
    virtual void update(GameObject* go) = 0;
    virtual void subscribeMessage(GameObject* go) = 0;
    virtual void handleMessage(Message* msg) = 0;

    // 如果 Component 全部都是使用 make_shared 创建的, 即使 Component 没有虚析构函数, 也可以安全地销毁
    // 因为 shared_ptr 会对 deleter 进行类型擦除. make_shared 会在构造时记住子类的 deleter
    // make_unique 和 new 创建的就会泄露
    virtual ~Component() = default;
};

struct Movable : MessageVisitor, Component {
    glm::vec3 position;
    glm::vec3 velocity;

    void handleMessage(Message* msg) override { msg->accept(this); }

    void visit(MoveMessage* mm) override { velocity += mm->velocityChange; }

    void visit(JumpMessage* jm) override { velocity.y += jm->jumpHeight; }
};

/*

当新增一种消息类型时, 需要修改的地方有:
    1. 新增消息类型
    2. 修改 MessageVisitor, 添加新的 visit 函数

当新增一种组件类型时, 需要修改的地方有:
    1. 新增组件类型

访问者模式通常用于 acceptor 数量有限, 但 visitor 的组件类型千变万化的情况
    如果消息类型有限, 组件类型可能经常增加, 把组件类型作为 visitor, 消息类型作为 acceptor
    如果组件类型有限, 消息类型可能经常增加, 把消息类型作为 visitor, 组件类型作为 acceptor
    常作为 acceptor 的有: 编译器开发中的 IR 节点, 游戏与 UI 开发中的消息类型
    常作为 visitor 的有: 编译器开发中的优化 pass, 游戏与 UI 开发中的j接收消息组件类型
*/

}  // namespace pyc
