module;

#include <entt/entity/registry.hpp>
#include <glm/glm.hpp>

module monster_war.game.system.health_bar_system;

import monster_war.engine.component.transform_component;
import monster_war.engine.render.camera;
import monster_war.engine.render.renderer;
import monster_war.engine.utils.math;
import monster_war.game.component.stats_component;
import monster_war.game.def.constants;
import monster_war.game.def.tag;

namespace pyc::monster_war {

void HealthBarSystem::update(entt::registry& registry, Renderer& renderer, Camera& camera) {
    // 只有受伤的实体才显示血量标签
    auto view = registry.view<TransformComponent, StatsComponent, HasHealthBarTag, InjuredTag>();

    for (auto [entity, transform, stats] : view.each()) {
        auto size = HEALTH_BAR_SIZE;
        // 血量条位置 = 角色位置 + 偏移量
        auto position = transform.position_ + glm::vec2(-size.x / 2.0f, HEALTH_BAR_OFFSET_Y);

        // 根据血量百分比确定颜色
        auto health_percent = stats.hp_ / stats.max_hp_;
        FColor color{};
        if (health_percent > 0.7f) {
            color = FColor::green();
        } else if (health_percent > 0.3f) {
            color = FColor::orange();
        } else {
            color = FColor::red();
        }

        // 执行绘制(先画边框，再画血量)
        renderer.drawRect(camera, position, size, color);
        size.x = size.x * health_percent;
        renderer.drawFilledRect(camera, position, size, color);
    }
}

}  // namespace pyc::monster_war
