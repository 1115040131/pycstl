#include "monster_war/game/system/orientation_system.h"

#include <entt/entity/registry.hpp>

#include "monster_war/engine/component/sprite_component.h"
#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/component/velocity_component.h"
#include "monster_war/game/component/blocked_by_component.h"
#include "monster_war/game/component/enemy_component.h"
#include "monster_war/game/component/target_component.h"
#include "monster_war/game/def/tag.h"

namespace pyc::monster_war {

void OrientationSystem::update(entt::registry& registry) {
    updateHasTarget(registry);
    updateBlocked(registry);
    updateMoving(registry);
}

void OrientationSystem::updateHasTarget(entt::registry& registry) {
    auto view_has_target = registry.view<TargetComponent, TransformComponent, SpriteComponent>();
    for (auto [entity, target, transform, sprite] : view_has_target.each()) {
        const auto& target_transform = registry.get<TransformComponent>(target.entity_);
        bool face_right = target_transform.position_.x > transform.position_.x;
        if (registry.all_of<FaceLeftTag>(entity)) {
            sprite.sprite_.is_flipped_ = face_right;
        } else {
            sprite.sprite_.is_flipped_ = !face_right;
        }
    }
}

void OrientationSystem::updateBlocked(entt::registry& registry) {
    auto view_blocked = registry.view<BlockedByComponent, TransformComponent, SpriteComponent>();
    for (auto [entity, blocked_by, transform, sprite] : view_blocked.each()) {
        const auto& blocker_transform = registry.get<TransformComponent>(blocked_by.entity_);
        bool face_right = blocker_transform.position_.x > transform.position_.x;
        if (registry.all_of<FaceLeftTag>(entity)) {
            sprite.sprite_.is_flipped_ = face_right;
        } else {
            sprite.sprite_.is_flipped_ = !face_right;
        }
    }
}

void OrientationSystem::updateMoving(entt::registry& registry) {
    auto view_moving = registry.view<VelocityComponent, EnemyComponent, SpriteComponent>(
        entt::exclude<BlockedByComponent, ActionLockTag>);
    for (auto [entity, velocity, _, sprite] : view_moving.each()) {
        bool face_right = velocity.velocity_.x > 0.0f;
        if (registry.all_of<FaceLeftTag>(entity)) {
            sprite.sprite_.is_flipped_ = face_right;
        } else {
            sprite.sprite_.is_flipped_ = !face_right;
        }
    }
}

}  // namespace pyc::monster_war