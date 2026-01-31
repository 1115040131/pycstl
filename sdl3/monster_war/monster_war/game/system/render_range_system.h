#pragma once

#include <entt/entity/fwd.hpp>

namespace pyc::monster_war {

class Renderer;
class Camera;

/**
 * @brief 渲染范围系统，根据条件渲染远程角色的攻击范围
 */
class RenderRangeSystem {
public:
    void update(entt::registry& registry, Renderer& renderer, const Camera& camera);
};

}  // namespace pyc::monster_war