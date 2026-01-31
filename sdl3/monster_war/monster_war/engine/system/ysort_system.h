#pragma once

#include <entt/entity/fwd.hpp>

namespace pyc::monster_war {

/**
 * @brief y-sort排序系统
 */
class YSortSystem {
public:
    void update(entt::registry& registry);
};

}  // namespace pyc::monster_war
