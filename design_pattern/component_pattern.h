#pragma once

#include <memory>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

namespace pyc {

struct Message {
    virtual ~Message() = default;
};

struct MoveMessage : Message {
    glm::vec3 velocityChange;
};

struct Component {
    virtual void update(GameObject* go) = 0;
    virtual void subscribeMessage(GameObject* go) = 0;
    virtual void handleMessage(Message* msg) = 0;

    // 如果 Component 全部都是使用 make_shared 创建的, 即使 Component 没有虚析构函数, 也可以安全地销毁
    // 因为 shared_ptr 会对 deleter 进行类型擦除. make_shared 会在构造时记住子类的 deleter
    // make_unique 和 new 创建的就会泄露
    virtual ~Component() = default;
};

struct GameObject {
    std::vector<std::unique_ptr<Component>> components;
    std::unordered_map<std::type_info, std::vector<Component*>> subscribers;  // 事件总线

    template <typename EventType>
    void subscribe(Component* component) {
        subscribers[typeid(EventType)].push_back(component);
    }

    template <typename EventType>
    void send(EventType* msg) {
        for (auto&& component : subscribers[typeid(EventType)]) {
            component->handleMessage(msg);
        }
    }

    void add(std::unique_ptr<Component> component) {
        components.push_back(std::move(component));
        component->subscribeMessage(this);
    }

    void update() {
        for (auto&& component : components) {
            component->update(this);
        }
    }

    void kill() {}
};

struct Movable : Component {
    glm::vec3 position;
    glm::vec3 velocity;

    void update(GameObject* go) override { position += velocity; }

    void subscribeMessage(GameObject* go) override { go->subscribe<MoveMessage>(this); }

    void handleMessage(Message* msg) override {
        if (auto moveMsg = dynamic_cast<MoveMessage*>(msg)) {
            velocity += moveMsg->velocityChange;
        }
    }
};

struct LivingBeging : Component {
    int ageLeft;

    void update(GameObject* go) override {
        if (ageLeft < 0) {
            go->kill();
        } else {
            ageLeft -= 1;
        }
    }

    void subscribeMessage(GameObject* go) override {}

    void handleMessage(Message* msg) override {}
};

struct PlayerController : Component {
    void update(GameObject* go) override {
        if (false /* 按键检测 */) {
            MoveMessage mm;
            mm.velocityChange.x += 1;
            go->send(&mm);
        }
    }

    void subscribeMessage(GameObject* go) override {}

    void handleMessage(Message* msg) override {}
};

struct PlayerAppearence : Component {
    void update(GameObject* go) override {
        // 渲染玩家
    }

    void subscribeMessage(GameObject* go) override {}

    void handleMessage(Message* msg) override {}
};

// 组件作为普通对象, 由 GameObject 构造函数创建
// struct Player : GameObject {
//     Movable* movable;
//     LivingBeging* living;
//     PlayerController* controller;
//     PlayerAppearence* appearence;

//     Player() {
//         movable = new Movable();
//         living = new LivingBeging();
//         controller = new PlayerController();
//         appearence = new PlayerAppearence();

//         add(std::unique_ptr<Movable>(movable));
//         add(std::unique_ptr<LivingBeging>(living));
//         add(std::unique_ptr<PlayerController>(controller));
//         add(std::unique_ptr<PlayerAppearence>(appearence));
//     }
// };

// 一个普通函数创建具有 Player 所需所有组件的 GameObject 对象
std::unique_ptr<GameObject> makePlayer() {
    auto go = std::make_unique<GameObject>();
    go->add(std::make_unique<Movable>());
    go->add(std::make_unique<LivingBeging>());
    go->add(std::make_unique<PlayerController>());
    go->add(std::make_unique<PlayerAppearence>());
    return go;
}

}  // namespace pyc
