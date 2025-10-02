#include "sunny_land/engine/component/health_component.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/object/game_object.h"

namespace pyc::sunny_land {

HealthComponent::HealthComponent(int max_health, std::chrono::duration<float> invincibility_duration)
    : max_health_(std::max(1, max_health)),
      current_health_(max_health),
      invincibility_duration_(invincibility_duration) {}

void HealthComponent::update(std::chrono::duration<float> delta_time, Context&) {
    // 更新无敌状态计时器
    if (is_invincible_) {
        invincibility_timer_ -= delta_time;
        if (invincibility_timer_.count() <= 0.0f) {
            is_invincible_ = false;
            invincibility_timer_ = {};
        }
    }
}

bool HealthComponent::takeDamage(int damage_amount) {
    if (damage_amount <= 0 || !isAlive()) {
        return false;
    }

    if (is_invincible_) {
        spdlog::debug("游戏对象 '{}' 处于无敌状态，免疫了 {} 点伤害。", owner_ ? owner_->getName() : "Unknown",
                      damage_amount);
        return false;  // 无敌状态，不受伤
    }

    current_health_ -= damage_amount;
    current_health_ = std::max(0, current_health_);
    if (isAlive() && invincibility_duration_.count() > 0.0f) {
        setInvincible(invincibility_duration_);
    }
    spdlog::debug("游戏对象 '{}' 受到了 {} 点伤害，当前生命值: {}/{}。", owner_ ? owner_->getName() : "Unknown",
                  damage_amount, current_health_, max_health_);
    return true;  // 造成伤害，返回true
}

int HealthComponent::heal(int heal_amount) {
    if (heal_amount <= 0 || !isAlive()) {
        return false;
    }

    current_health_ += heal_amount;
    current_health_ = std::min(current_health_, max_health_);
    spdlog::debug("游戏对象 '{}' 治疗了 {} 点，当前生命值: {}/{}。", owner_ ? owner_->getName() : "Unknown",
                  heal_amount, current_health_, max_health_);
    return current_health_;
}

void HealthComponent::setCurrentHealth(int current_health) {
    current_health_ = std::clamp(current_health, 0, max_health_);
}

void HealthComponent::setMaxHealth(int max_health) {
    max_health_ = std::max(1, max_health);
    current_health_ = std::min(current_health_, max_health_);
}

void HealthComponent::setInvincible(std::chrono::duration<float> duration) {
    if (duration.count() > 0.0f) {
        is_invincible_ = true;
        invincibility_timer_ = duration;
        spdlog::debug("游戏对象 '{}' 进入无敌状态，持续 {} 秒。", owner_ ? owner_->getName() : "Unknown",
                      duration.count());
    } else {
        // 如果持续时间为 0 或负数，则立即取消无敌
        is_invincible_ = false;
        invincibility_timer_ = {};
        spdlog::debug("游戏对象 '{}' 的无敌状态被手动移除。", owner_ ? owner_->getName() : "Unknown");
    }
}

}  // namespace pyc::sunny_land