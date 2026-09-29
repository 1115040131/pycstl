module;

#include <entt/entity/entity.hpp>

export module monster_war.game.component.target_component;

export namespace pyc::monster_war {

/**
 * @brief 目标组件，包含目标实体，用于锁定攻击对象。
 */
struct TargetComponent {
    entt::entity entity_{entt::null};
};

}  // namespace pyc::monster_war
