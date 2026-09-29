module;

#include <entt/entity/entity.hpp>

export module monster_war.game.component.blocked_by_component;

export namespace pyc::monster_war {

/// @brief 被阻挡组件，存储自身被哪个阻挡者阻挡（敌方单位用）
struct BlockedByComponent {
    entt::entity entity_{entt::null};
};

}  // namespace pyc::monster_war
