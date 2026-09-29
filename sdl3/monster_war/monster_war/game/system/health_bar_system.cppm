module;

#include <entt/entity/fwd.hpp>

export module monster_war.game.system.health_bar_system;

export import monster_war.engine.render.camera;
export import monster_war.engine.render.renderer;

export namespace pyc::monster_war {

/**
 * @brief 地图血量条系统(渲染)，用于显示角色的血量条
 */
class HealthBarSystem {
public:
    void update(entt::registry& registry, Renderer& renderer, Camera& camera);
};

}  // namespace pyc::monster_war
