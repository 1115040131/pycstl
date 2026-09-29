module;

#include <entt/entity/fwd.hpp>

export module monster_war.engine.system.ysort_system;

export namespace pyc::monster_war {

/**
 * @brief y-sort排序系统
 */
class YSortSystem {
public:
    void update(entt::registry& registry);
};

}  // namespace pyc::monster_war
