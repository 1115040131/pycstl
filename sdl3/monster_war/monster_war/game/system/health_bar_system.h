#pragma once

#include <entt/entity/fwd.hpp>

namespace pyc::monster_war {

class Renderer;
class Camera;

/**
 * @brief 地图血量条系统(渲染)，用于显示角色的血量条
 */
class HealthBarSystem {
public:
    void update(entt::registry& registry, Renderer& renderer, Camera& camera);
};

}  // namespace pyc::monster_war