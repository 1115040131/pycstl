module;

#include <entt/entity/fwd.hpp>

export module monster_war.game.system.remove_dead_system;

export namespace pyc::monster_war {

/**
 * @brief 清理死亡实体的系统
 */
class RemoveDeadSystem {
public:
    void update(entt::registry& registry);
};

}  // namespace pyc::monster_war
