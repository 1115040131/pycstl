#include "monster_war/engine/system/render_system.h"

#include <entt/entity/registry.hpp>

#include "monster_war/engine/component/render_component.h"
#include "monster_war/engine/component/sprite_component.h"
#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/render/renderer.h"

namespace pyc::monster_war {

void RenderSystem::update(entt::registry& registry, Renderer& renderer, const Camera& camera) {
    // 对 RenderComponent 进行排序 (需要自定义 RenderComponent 的比较运算符)
    registry.sort<RenderComponent>([](const auto& lhs, const auto& rhs) { return lhs < rhs; });

    // 执行渲染，注意排序组件 RenderComponent 必须放在最前面
    auto view = registry.view<RenderComponent, TransformComponent, SpriteComponent>();
    for (auto [_, __, transform, sprite] : view.each()) {
        auto position = transform.position_ + sprite.offset_;
        auto size = sprite.size_ * transform.scale_;
        renderer.drawSprite(camera, sprite.sprite_, position, size);
    }
}

}  // namespace pyc::monster_war
