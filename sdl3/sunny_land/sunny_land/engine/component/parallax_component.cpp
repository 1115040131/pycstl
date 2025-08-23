#include "sunny_land/engine/component/parallax_component.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/render/renderer.h"

namespace pyc::sunny_land {

ParallaxComponent::ParallaxComponent(std::string_view texture_id, glm::vec2 scroll_factor, glm::bvec2 repeat)
    : sprite_(texture_id), scroll_factor_(std::move(scroll_factor)), repeat_(std::move(repeat)) {
    spdlog::trace("ParallaxComponent 初始化完成，纹理 ID: {}", texture_id);
}

void ParallaxComponent::init() {
    if (!owner_) {
        spdlog::error("ParallaxComponent 在初始化前未设置所有者。");
        return;
    }
    transform_component_ = owner_->getComponent<TransformComponent>();
    if (!transform_component_) {
        spdlog::warn("GameObject '{}' 上的 ParallaxComponent 需要一个 TransformComponent, 但未找到。",
                     owner_->getName());
        return;
    }
}

void ParallaxComponent::render(Context& context) {
    if (is_hidden_ || !transform_component_) {
        return;
    }

    context.getRenderer().drawParallax(context.getCamera(), sprite_, transform_component_->getPosition(),
                                       scroll_factor_, repeat_);
}

}  // namespace pyc::sunny_land