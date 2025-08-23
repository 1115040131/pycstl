#include "sunny_land/engine/component/sprite_component.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/render/renderer.h"
#include "sunny_land/engine/resource/resource_manager.h"

namespace pyc::sunny_land {

SpriteComponent::SpriteComponent(Sprite&& sprite, ResourceManager& resource_manager, Alignment alignment)
    : resource_manager_(&resource_manager), sprite_(std::move(sprite)), alignment_(alignment) {
    if (!resource_manager_) {
        spdlog::critical("创建 SpriteComponent 时 ResourceManager 为空！，此组件将无效。");
        // 不要在游戏主循环中使用 try...catch / throw，会极大影响性能
    }
    // offset_ 和 sprite_size_ 将在 init 中计算
    spdlog::trace("创建 SpriteComponent, 纹理ID: {}", sprite_.getTextureId());
}

SpriteComponent::SpriteComponent(std::string_view texture_id, ResourceManager& resource_manager,
                                 Alignment alignment, std::optional<SDL_FRect> source_rect_opt, bool is_flipped)
    : SpriteComponent(Sprite(texture_id, std::move(source_rect_opt), is_flipped), resource_manager, alignment) {}

void SpriteComponent::setSpriteById(std::string_view texture_id, std::optional<SDL_FRect> source_rect_opt) {
    sprite_.setTextureId(texture_id);
    sprite_.setSourceRect(std::move(source_rect_opt));

    updateSpriteSize();
    updateOffset();
}

void SpriteComponent::setSourceRect(std::optional<SDL_FRect> source_rect_opt) {
    sprite_.setSourceRect(std::move(source_rect_opt));

    updateSpriteSize();
    updateOffset();
}

void SpriteComponent::setAlignment(Alignment anchor) {
    alignment_ = anchor;
    updateOffset();
}

void SpriteComponent::updateSpriteSize() {
    if (const auto& src_rect = sprite_.getSourceRect()) {
        sprite_size_ = glm::vec2(src_rect->w, src_rect->h);
    } else {
        if (!resource_manager_) {
            spdlog::error("ResourceManager 为空！无法获取纹理尺寸。");
            return;
        }
        sprite_size_ = resource_manager_->getTextureSize(sprite_.getTextureId());
    }
}

void SpriteComponent::updateOffset() {
    // 如果尺寸无效，偏移为0
    if (sprite_size_.x <= 0 || sprite_size_.y <= 0) {
        offset_ = {0.0f, 0.0f};
        return;
    }
    const auto& scale = transform_component_->getScale();
    // 计算精灵左上角相对于 TransformComponent::position_ 的偏移
    switch (alignment_) {
        case Alignment::TOP_LEFT:
            offset_ = glm::vec2{0.0f, 0.0f} * scale;
            break;
        case Alignment::TOP_CENTER:
            offset_ = glm::vec2{-sprite_size_.x / 2.0f, 0.0f} * scale;
            break;
        case Alignment::TOP_RIGHT:
            offset_ = glm::vec2{-sprite_size_.x, 0.0f} * scale;
            break;
        case Alignment::CENTER_LEFT:
            offset_ = glm::vec2{0.0f, -sprite_size_.y / 2.0f} * scale;
            break;
        case Alignment::CENTER:
            offset_ = glm::vec2{-sprite_size_.x / 2.0f, -sprite_size_.y / 2.0f} * scale;
            break;
        case Alignment::CENTER_RIGHT:
            offset_ = glm::vec2{-sprite_size_.x, -sprite_size_.y / 2.0f} * scale;
            break;
        case Alignment::BOTTOM_LEFT:
            offset_ = glm::vec2{0.0f, -sprite_size_.y} * scale;
            break;
        case Alignment::BOTTOM_CENTER:
            offset_ = glm::vec2{-sprite_size_.x / 2.0f, -sprite_size_.y} * scale;
            break;
        case Alignment::BOTTOM_RIGHT:
            offset_ = glm::vec2{-sprite_size_.x, -sprite_size_.y} * scale;
            break;
        case Alignment::NONE:
        default:
            break;
    }
}

void SpriteComponent::init() {
    if (!owner_) {
        spdlog::error("SpriteComponent 在初始化前未设置所有者。");
        return;
    }
    transform_component_ = owner_->getComponent<TransformComponent>();
    if (!transform_component_) {
        spdlog::warn("GameObject '{}' 上的 SpriteComponent 需要一个 TransformComponent, 但未找到。",
                     owner_->getName());
        // Sprite没有Transform无法计算偏移和渲染，直接返回
        return;
    }

    // 获取大小及偏移
    updateSpriteSize();
    updateOffset();
}

void SpriteComponent::render(Context& context) {
    if (is_hidden_ || !transform_component_ || !resource_manager_) {
        return;
    }

    // 获取变换信息（考虑偏移量）
    const glm::vec2& pos = transform_component_->getPosition() + offset_;
    const glm::vec2& scale = transform_component_->getScale();
    float rotation_degrees = transform_component_->getRotation();

    // 执行绘制
    context.getRenderer().drawSprite(context.getCamera(), sprite_, pos, scale, rotation_degrees);
}

}  // namespace pyc::sunny_land