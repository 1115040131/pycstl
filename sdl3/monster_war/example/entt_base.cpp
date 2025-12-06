#include <entt/entt.hpp>
#include <fmt/base.h>
#include <glm/glm.hpp>

struct position {
    float x;
    float y;
};

struct velocity {
    float dx;
    float dy;
};

// 增加一个新组件, 用于标记静态物体
struct immovable_tag {};

// 增加一个组件来存储实体的 "标签"
struct tag {
    // entt::hashed_string value;  // 只保留字符串的指针, 不保证字符串是否被销毁, 将来使用 value.data() 可能为空

    // 确保字符串不被销毁的正确做法: value 用于储存, id 用于比较/查询
    entt::id_type id;
    std::string value;
};

void add_context(entt::registry& registry) {
    using namespace entt::literals;

    // 可以在这里添加全局上下文数据, 例如游戏设置、资源管理器等
    registry.ctx().emplace<bool>(true);
    registry.ctx().emplace<glm::vec2>(1.0f, 2.0f);
    registry.ctx().emplace<std::string>("hello");
    // emplace 每种类型只能有一个实例,  emplace_as 可以添加多个不同哈希标识的实例
    registry.ctx().emplace_as<std::string>("hello"_hs, "hello");
}

void get_context(entt::registry& registry) {
    using namespace entt::literals;

    fmt::println("=== 获取全局上下文数据 ===");

    bool flag = registry.ctx().get<bool>();
    fmt::println("Context bool: {}", flag);

    glm::vec2 vec = registry.ctx().get<glm::vec2>();
    fmt::println("Context glm::vec2: ({}, {})", vec.x, vec.y);

    std::string str = registry.ctx().get<std::string>();
    fmt::println("Context std::string: {}", str);

    std::string str2 = registry.ctx().get<std::string>("hello"_hs);
    fmt::println("Context std::string with hash 'hello': {}", str);
}

void entt_base() {
    using namespace entt::literals;

    // 1. 创建 Registry
    // Registry 是所有实体和组件的容器, 可以看作是"游戏世界"
    entt::registry registry;

    // 2. 创建实体
    // 实体本身是一个唯一标识符 (ID)
    entt::entity player = registry.create();

    // 3. 向实体添加组件
    // 使用 emplace<ComponentType>(entity, args...) 方法向实体添加组件
    registry.emplace<position>(player, 10.f, 20.f);
    registry.emplace<velocity>(player, 1.f, 0.f);

    entt::entity enemy = registry.create();
    registry.emplace<position>(enemy, 100.f, 50.f);
    registry.emplace<velocity>(enemy, -0.5f, -1.f);

    entt::entity obstacle = registry.create();
    registry.emplace<position>(obstacle, 200.f, 200.f);
    registry.emplace<immovable_tag>(obstacle);  // 障碍物没有速度, 不可移动

    // a.1. 使用哈希字符串作为组件数据
    registry.emplace<tag>(player, "player"_hs, "player");
    // registry.emplace<tag>(enemy, "enemy"_hs);
    // 运行时转换
    auto enemy_name = "enemy";
    auto enemy_hs = entt::hashed_string{enemy_name};
    registry.emplace<tag>(enemy, enemy_hs, enemy_name);

    fmt::println("=== 使用哈希字符串标签 ===");

    // a.2. 通过视图遍历并识别实体
    {
        auto view = registry.view<const tag, const position>();
        view.each([](const auto& tag, const auto& pos) {
            if (tag.id == "player"_hs) {
                fmt::println("找到玩家, 位置: ({}, {})", pos.x, pos.y);
            } else if (tag.id == "enemy"_hs) {
                fmt::println("找到敌人, 位置: ({}, {})", pos.x, pos.y);
            }
        });
    }

    // a.3. 哈希字符串的哈希值与原始值
    auto player_tag = registry.get<tag>(player);
    fmt::println("玩家标签哈希值: {}, 原始字符串哈希值: {}", player_tag.id, player_tag.value);

    // 4. 修改组件
    // 获取组件引用
    // auto& player_pos = registry.get<position>(player);
    // fmt::println("玩家位置: ({}, {})", player_pos.x, player_pos.y);
    // player_pos.x += 5.f;  // 移动玩家
    // fmt::println("玩家新位置: ({}, {})", player_pos.x, player_pos.y);

    fmt::println("=== 游戏 update 更新前 ===");
    auto initial_player_pos = registry.get<position>(player);
    fmt::println("玩家初始位置: ({}, {})", initial_player_pos.x, initial_player_pos.y);
    auto initial_enemy_pos = registry.get<position>(enemy);
    fmt::println("敌人初始位置: ({}, {})", initial_enemy_pos.x, initial_enemy_pos.y);

    // === 游戏更新 (通常在 System 中进行) ===
    // b.1. 创建一个视图
    // 创建一个包含所有 *同时* 拥有 position 和 velocity 组件的实体的视图
    // 障碍物(obstacle) 没有 velocity 组件, 因此不会包含在视图中
    {
        auto view = registry.view<position, velocity>();
        fmt::println("--- 正在更新 {} 个可移动实体 ---", view.size_hint());

        // b.2. 遍历视图并更新位置
        for (auto entity : view) {
            auto& pos = view.get<position>(entity);
            const auto& vel = view.get<velocity>(entity);
            pos.x += vel.dx;
            pos.y += vel.dy;
        }
    }

    fmt::println("=== 游戏 update 更新后 ===");
    auto final_player_pos = registry.get<position>(player);
    fmt::println("玩家更新后位置: ({}, {})", final_player_pos.x, final_player_pos.y);
    auto final_enemy_pos = registry.get<position>(enemy);
    fmt::println("敌人初始位置: ({}, {})", final_enemy_pos.x, final_enemy_pos.y);

    // 5. 移除组件
    // 使用 remove<ComponentType>(entity) 方法移除组件
    registry.remove<velocity>(player);

    // 6. 销毁实体
    // 销毁实体会自动移除其所有组件
    registry.destroy(enemy);

    // 检查实体是否存在
    if (registry.valid(player)) {
        fmt::println("玩家实体仍然存在");
    }
    if (!registry.valid(enemy)) {
        fmt::println("敌人实体已被销毁");
    }

    add_context(registry);
    get_context(registry);
}