#include <fmt/base.h>
#include <glm/glm.hpp>

#include "entt_struct.h"

// 一个符合委托签名的全局函数
int multiply_by_two(int value) {
    fmt::println("[全局函数] multiply_by_two({}) 被调用", value);
    return value * 2;
}

// 一个普通的类, 使用它的成员函数
class Calculator {
public:
    explicit Calculator(int factor) : multiplication_factor(factor) {}

    int multiply(int value) {
        fmt::println("[成员函数] Calculator::multiply({}) 被调用", value);
        return value * multiplication_factor;
    }

private:
    int multiplication_factor;
};

void entt_delegate() {
    // 委托(entt::delegate)
    // std::function 的轻量级, 高性能替代品, 专用于单播回调
    // 主要缺陷: lambda 表达式支持不完整

    // 1. 定义一个委托
    entt::delegate<int(int)> delegate;

    // 2. 检查委托是否有效
    if (!delegate) {
        fmt::println("委托当前无效");
    }

    // 3. 连接一个全局函数
    delegate.connect<&multiply_by_two>();

    // 调用委托
    if (delegate) {
        int result = delegate(10);
        fmt::println("调用委托后的结果: {}", result);
    }

    // 4. 连接一个成员函数
    Calculator calculator(5);
    delegate.connect<&Calculator::multiply>(&calculator);

    // 调用委托
    if (delegate) {
        int result = delegate(10);
        fmt::println("调用委托后的结果: {}", result);
    }

    // 5. 重置委托
    delegate.reset();
    if (!delegate) {
        fmt::println("委托已被重置, 当前无效");
    }

    // 6. 使用构造函数直接连接
    Calculator other_calculator(3);
    entt::delegate<int(int)> another_delegate{entt::connect_arg<&Calculator::multiply>, &other_calculator};

    // 调用新的委托
    if (another_delegate) {
        int result = another_delegate(10);
        fmt::println("调用另一个委托后的结果: {}", result);
    }
}

// 普通全局函数, 可以作为监听器, 签名必须与信号匹配
void free_function_listener(int value, std::string_view message) {
    fmt::println("[全局函数] free_function_listener({}, {}) 被调用", value, message);
}

// 监听器类
class Listener {
public:
    void on_value_changed(int value, std::string_view message) {
        fmt::println("[成员函数] Listener::on_value_changed({}, {}) 被调用", value, message);
        data_++;
    }

    int data_{};
};

void entt_signal() {
    // 信号(entt::sigh, ent::sink)
    // 多播的 std::function, 一个信号可以连接多个监听者

    // entt::sigh 信号本身, 负责存储所有监听者并发布时间
    // entt::sink 信号的入口, 提供了一个安全的接口来连接和断开监听器

    // 1. 定义一个信号
    entt::sigh<void(int, std::string_view)> on_change;

    // 2. 创建一个 sink 管理连接
    entt::sink sink{on_change};

    // 创建一个监听器实例
    Listener listener_instance{};

    fmt::println("=== 连接监听器 ===");

    // 3. 连接回调
    // a) 连接全局函数
    sink.connect<&free_function_listener>();
    // b) 连接成员函数
    sink.connect<&Listener::on_value_changed>(&listener_instance);
    // c) 连接 lambda 表达式 (注意: 不支持捕获变量)
    sink.connect<[](int value, std::string_view message) {
        fmt::println("[Lambda] 被调用 with ({}, {})", value, message);
    }>();

    fmt::println("当前连接 {} 个监听器", on_change.size());

    // 4. 发布信号
    // 使用 publish() 来触发信号
    fmt::println("=== 第一次发布信号 ===");
    on_change.publish(42, "Hello World");
    fmt::println("Listener 实例中的数据: {}", listener_instance.data_);

    // 5. 断开连接
    fmt::println("=== 断开成员函数监听器 ===");
    sink.disconnect<&Listener::on_value_changed>(&listener_instance);
    fmt::println("当前连接 {} 个监听器", on_change.size());

    // 6. 再次发布信号
    fmt::println("=== 第二次发布信号 ===");
    on_change.publish(99, "Goodbye");
    fmt::println("Listener 实例中的数据: {}", listener_instance.data_);
}

// 上下文变量
struct game_state {
    int score{};
};

// 1. 定义事件
// 事件通常是简单的结构体, 用于携带数据
struct enemy_destroyed_event {
    entt::entity enemy_entity;
};

// 2. 创建一个监听器
class ScoreSystem {
public:
    explicit ScoreSystem(entt::registry& registry) : registry_(registry) {}

    void on_enemy_destroyed(const enemy_destroyed_event& event) {
        // 增加分数
        auto& state = registry_.ctx().get<game_state>();
        state.score += 10;

        fmt::println("[成员函数] ScoreSystem::on_enemy_destroyed({}) 被调用, 当前分数: {}",
                     entt::to_integral(event.enemy_entity), state.score);
    }

private:
    entt::registry& registry_;
};

// 另一种监听器
void dummy_listener(const enemy_destroyed_event& event) {
    fmt::println("[全局函数] dummy_listener({}) 被调用", entt::to_integral(event.enemy_entity));
}

void entt_dispatcher() {
    // 分发器(entt::dispatcher)
    // 事件中心, 集中管理各类时间信号与回调, 支持事件队列

    using namespace entt::literals;
    entt::registry registry;

    // 注册上下文变量
    registry.ctx().emplace<game_state>();

    // 3. 创建事件分发器和监听器实例
    entt::dispatcher dispatcher;
    ScoreSystem score_system{registry};

    // 4. 连接监听器
    dispatcher.sink<enemy_destroyed_event>().connect<&ScoreSystem::on_enemy_destroyed>(&score_system);

    // 同一个事件可以连接多个函数 (注意调用顺序, 后进先调)
    dispatcher.sink<enemy_destroyed_event>().connect<&dummy_listener>();

    // 创建实体
    entt::entity enemy = registry.create();
    registry.emplace<tag>(enemy, "enemy"_hs, "enemy");

    fmt::println("=== 游戏进行中 ===");
    fmt::println("当前分数: {}", registry.ctx().get<game_state>().score);

    // ... 战斗发生 ...
    fmt::println("玩家摧毁了敌人 {}!", static_cast<uint32_t>(enemy));
    registry.destroy(enemy);

    // 5. 发布事件
    //  enqueue 会将事件加入队列, 在循环末尾统一处理 (trigger 会立即触发)
    dispatcher.enqueue(enemy_destroyed_event{enemy});

    // 6. 更新分发器
    // 处理所有排队的事件
    dispatcher.update();
}