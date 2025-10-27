#include "sunny_land/engine/component/collider_component.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/physics/collision.h"

namespace pyc::sunny_land {

ColliderComponent::ColliderComponent(std::unique_ptr<Collider> collider, Alignment alignment, bool is_trigger,
                                     bool is_active)
    : collider_(std::move(collider)), alignment_(alignment), is_trigger_(is_trigger), is_active_(is_active) {
    if (!collider_) {
        spdlog::error("创建 ColliderComponent 时传入了空的碰撞器！");
    }
}

void ColliderComponent::init() {
    if (!owner_) {
        spdlog::error("ColliderComponent 没有所有者 GameObject！");
        return;
    }
    transform_ = owner_->getComponent<TransformComponent>();
    if (!transform_) {
        spdlog::error("ColliderComponent 需要一个在同一个 GameObject 上的 TransformComponent！");
        return;
    }

    updateOffset();
}

void ColliderComponent::setAlignment(Alignment anchor) {
    alignment_ = anchor;
    updateOffset();
}

void ColliderComponent::updateOffset() {
    if (!collider_) {
        return;
    }

    // 获取碰撞盒的最小包围盒尺寸
    const auto& collider_size = collider_->getAABBSize();

    // 如果尺寸无效，偏移为0
    if (collider_size.x <= 0.0f || collider_size.y <= 0.0f) {
        offset_ = {0.0f, 0.0f};
        return;
    }

    const auto& scale = transform_->getScale();
    // 根据 alignment_anchor_ 计算 AABB 左上角相对于 Transform 中心的偏移量
    switch (alignment_) {
        case Alignment::TOP_LEFT:
            offset_ = glm::vec2{0.0f, 0.0f} * scale;
            break;
        case Alignment::TOP_CENTER:
            offset_ = glm::vec2{-collider_size.x / 2.0f, 0.0f} * scale;
            break;
        case Alignment::TOP_RIGHT:
            offset_ = glm::vec2{-collider_size.x, 0.0f} * scale;
            break;
        case Alignment::CENTER_LEFT:
            offset_ = glm::vec2{0.0f, -collider_size.y / 2.0f} * scale;
            break;
        case Alignment::CENTER:
            offset_ = glm::vec2{-collider_size.x / 2.0f, -collider_size.y / 2.0f} * scale;
            break;
        case Alignment::CENTER_RIGHT:
            offset_ = glm::vec2{-collider_size.x, -collider_size.y / 2.0f} * scale;
            break;
        case Alignment::BOTTOM_LEFT:
            offset_ = glm::vec2{0.0f, -collider_size.y} * scale;
            break;
        case Alignment::BOTTOM_CENTER:
            offset_ = glm::vec2{-collider_size.x / 2.0f, -collider_size.y} * scale;
            break;
        case Alignment::BOTTOM_RIGHT:
            offset_ = glm::vec2{-collider_size.x, -collider_size.y} * scale;
            break;
        default:
            break;  // 如果 alignment_ 是 NONE，则不做任何操作（手动设置 offset_）
    }
}

Rect ColliderComponent::getWorldAABB() const {
    if (!transform_ || !collider_) {
        return Rect{};
    }
    return Rect{
        transform_->getPosition() + offset_,
        collider_->getAABBSize() * transform_->getScale(),
    };
}

}  // namespace pyc::sunny_land