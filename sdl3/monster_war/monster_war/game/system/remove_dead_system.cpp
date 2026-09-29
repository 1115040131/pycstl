module;

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

module monster_war.game.system.remove_dead_system;

import monster_war.game.def.tag;

namespace pyc::monster_war {

void RemoveDeadSystem::update(entt::registry& registry) {
    // 标签本质上是空的组件
    auto view = registry.view<DeadTag>();
    for (auto entity : view) {
        registry.destroy(entity);
        spdlog::info("RemoveDeadSystem::update 清理了死亡实体: {}", entt::to_integral(entity));
    }
}

}  // namespace pyc::monster_war
