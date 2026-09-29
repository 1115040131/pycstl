module;

#include <entt/entity/registry.hpp>

module monster_war.game.system.render_range_system;

import monster_war.engine.component.transform_component;
import monster_war.engine.render.camera;
import monster_war.engine.render.renderer;
import monster_war.game.component.stats_component;
import monster_war.game.component.unit_prep_component;
import monster_war.game.def.tag;

namespace pyc::monster_war {

void RenderRangeSystem::update(entt::registry& registry, Renderer& renderer, const Camera& camera) {
    // 准备放置类型的单位
    auto view_prep = registry.view<ShowRangeTag, TransformComponent, UnitPrepComponent>();
    for (auto [entity, transform, unit_prep] : view_prep.each()) {
        // 攻击范围显示为透明绿色圆形
        renderer.drawFilledCircle(camera, transform.position_, unit_prep.range_, RANGE_COLOR);
    }
    // 地图上的单位
    auto view_remote = registry.view<ShowRangeTag, TransformComponent, StatsComponent>();
    for (auto [entity, transform, stats] : view_remote.each()) {
        // 攻击范围显示为透明绿色圆形
        renderer.drawFilledCircle(camera, transform.position_, stats.range_, RANGE_COLOR);
    }
}

}  // namespace pyc::monster_war
