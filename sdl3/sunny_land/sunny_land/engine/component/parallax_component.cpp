module;

#include <string_view>
#include <utility>

#include <SDL3/SDL_rect.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

module sunny_land.engine.component.parallax_component;

import sunny_land.engine.core;
import sunny_land.engine.render.sprite;

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
    transform_ = owner_->getComponent<TransformComponent>();
    if (!transform_) {
        spdlog::warn("GameObject '{}' 上的 ParallaxComponent 需要一个 TransformComponent, 但未找到。",
                     owner_->getName());
        return;
    }
}

void ParallaxComponent::render(Context& context) {
    if (is_hidden_ || !transform_) {
        return;
    }

    context.getRenderer().drawParallax(context.getCamera(), sprite_, transform_->getPosition(), scroll_factor_,
                                       repeat_);
}

}  // namespace pyc::sunny_land
