module;

#include <entt/entity/fwd.hpp>

export module monster_war.game.system.render_range_system;

export import monster_war.engine.render.camera;
export import monster_war.engine.render.renderer;

export namespace pyc::monster_war {

/**
 * @brief 渲染范围系统，根据条件渲染远程角色的攻击范围
 */
class RenderRangeSystem {
public:
    void update(entt::registry& registry, Renderer& renderer, const Camera& camera);
};

}  // namespace pyc::monster_war
