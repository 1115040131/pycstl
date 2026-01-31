#pragma once

#include <unordered_map>

#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

#include "monster_war/game/data/waypoint_node.h"

namespace pyc::monster_war {

/**
 * @brief 路径跟随系统。
 * 根据路径节点更新敌人实体的速度和目标节点。
 */
class FollowPathSystem {
public:
    void update(entt::registry& registry, entt::dispatcher& dispatcher,
                std::unordered_map<int, WaypointNode>& waypoint_nodes);
};

}  // namespace pyc::monster_war