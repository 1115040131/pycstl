#include "monster_war/game/system/render_range_system.h"

#include <entt/entity/registry.hpp>

#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/render/camera.h"
#include "monster_war/engine/render/renderer.h"
#include "monster_war/game/component/unit_prep_component.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

void RenderRangeSystem::update(entt::registry& registry, Renderer& renderer, const Camera& camera) {
    // 准备放置类型的单位
    auto view_prep = registry.view<ShowRangeTag, TransformComponent, UnitPrepComponent>();
    for (auto [entity, transform, unit_prep] : view_prep.each()) {
        // 攻击范围显示为透明绿色圆形
        renderer.drawFilledCircle(camera, transform.position_, unit_prep.range_, RANGE_COLOR);
    }
    // TODO: 地图上的远程单位
}

}  // namespace pyc::monster_war