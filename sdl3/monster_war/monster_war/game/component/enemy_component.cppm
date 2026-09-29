module;

#include <glm/vec2.hpp>

export module monster_war.game.component.enemy_component;

export namespace pyc::monster_war {

/**
 * @brief 敌人组件，包含目标节点ID和自身速度。
 */
struct EnemyComponent {
    int target_waypoint_id_;
    float speed_;
};

}  // namespace pyc::monster_war
