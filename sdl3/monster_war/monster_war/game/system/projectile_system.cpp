#include "monster_war/game/system/projectile_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/utils/events.h"
#include "monster_war/game/component/projectile_component.h"
#include "monster_war/game/def/tag.h"
#include "monster_war/game/factory/entity_factory.h"

namespace pyc::monster_war {

using namespace entt::literals;

ProjectileSystem::ProjectileSystem(entt::registry& registry, entt::dispatcher& dispatcher,
                                   EntityFactory& entity_factory)
    : registry_(registry), dispatcher_(dispatcher), entity_factory_(entity_factory) {
    dispatcher_.sink<EmitProjectileEvent>().connect<&ProjectileSystem::onEmitProjectileEvent>(this);
}

ProjectileSystem::~ProjectileSystem() { dispatcher_.disconnect(this); }

void ProjectileSystem::update(std::chrono::duration<float> delta_time) {
    // 获取所有投射物
    auto view = registry_.view<ProjectileComponent, TransformComponent>();
    for (auto [entity, projectile, transform] : view.each()) {
        // 更新飞行时间
        projectile.current_flight_time_ += delta_time;
        // 如果飞行时间超过总飞行时间，则命中目标（发送攻击事件以及播放音效）并销毁
        if (projectile.current_flight_time_ >= projectile.total_flight_time_) {
            dispatcher_.enqueue(AttackEvent{entity, projectile.target_, projectile.damage_});
            dispatcher_.enqueue(PlaySoundEvent{entity, "hit"_hs});
            registry_.emplace<DeadTag>(entity);
            continue;
        }

        // 计算飞行进度 (t 从 0 到 1)
        auto t = projectile.current_flight_time_.count() / projectile.total_flight_time_.count();
        t = std::clamp(t, 0.0f, 1.0f);

        // 1. 计算水平位置 (线性插值)
        glm::vec2 horizontal_pos = glm::mix(projectile.start_position_, projectile.target_position_, t);

        // 2. 计算垂直方向的弧线偏移
        // 使用 sin 函数可以轻松创建弧线: sin(0)=0, sin(PI/2)=1, sin(PI)=0
        float arc_offset = glm::sin(t * std::numbers::pi) * projectile.arc_height_;

        // 3. 合成最终位置
        transform.position_ = horizontal_pos;
        transform.position_.y -= arc_offset;  // Y轴向下为正，所以减去偏移使其向上拱起

        // 4. 根据上一帧的位置计算朝向，并更新TransformComponent的旋转参数
        auto direction = transform.position_ - projectile.previous_position_;
        transform.rotation_ = glm::atan(direction.y, direction.x) * 180.0f / std::numbers::pi;

        spdlog::info("({:.2f}, {:.2f}) -> ({:.2f}, {:.2f}) direction: ({:.2f}, {:.2f}), rotation: {}",
                     projectile.previous_position_.x, projectile.previous_position_.y, transform.position_.x,
                     transform.position_.y, direction.x, direction.y, transform.rotation_);

        // 5. 更新上一帧的位置
        projectile.previous_position_ = transform.position_;
    }
}

void ProjectileSystem::onEmitProjectileEvent(const EmitProjectileEvent& event) {
    spdlog::info("发射投射物: {}", event.id_);
    entity_factory_.createProjectile(event.id_, event.start_position_, event.target_position_, event.target_,
                                     event.damage_);
}

}  // namespace pyc::monster_war