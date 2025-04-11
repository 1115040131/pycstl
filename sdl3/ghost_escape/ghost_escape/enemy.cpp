#include "ghost_escape/enemy.h"

#include "sdl3/common/raw/stats.h"

namespace pyc {
namespace sdl3 {

std::unique_ptr<Enemy> Enemy::Create(const glm::vec2& position, Actor* target) {
    auto enemy = std::make_unique<Enemy>();
    enemy->init();
    enemy->setPosition(position);
    enemy->setTarget(target);
    return enemy;
}

void Enemy::init() {
    Actor::init();

#ifdef DEBUG_MODE
    SET_NAME(Enemy);
#endif

    type_ = Object::Type::kEnemy;

    anim_normal_ = SpriteAnim::CreateAndSet(this, ASSET("sprite/ghost-Sheet.png"), 2.F);
    anim_hurt_ = SpriteAnim::CreateAndSet(this, ASSET("sprite/ghostHurt-Sheet.png"), 2.F);
    anim_die_ = SpriteAnim::CreateAndSet(this, ASSET("sprite/ghostDead-Sheet.png"), 2.F, 10.F, false);
    anim_hurt_->setActive(false);
    anim_die_->setActive(false);
    current_anim_ = anim_normal_;

    collider_ = Collider::CreateAndSet(this, current_anim_->getSize());

    stats_ = Stats::CreateAndSet(this);
}

void Enemy::update(std::chrono::duration<float> delta) {
    if (target_ && target_->isAlive() && isAlive()) {
        aimTarget();
        move(delta);
        attack();
    }
    checkState();
    remove();
    Actor::update(delta);
}

void Enemy::aimTarget() {
    if (target_ == nullptr) {
        return;
    }
    auto direction = target_->getPosition() - this->getPosition();
    if (glm::length(direction) < 1.F) {
        velocity_ = glm::vec2(0.F);
        return;
    }
    velocity_ = glm::normalize(direction) * max_speed_;
}

void Enemy::attack() {
    if (!collider_ || !target_ || !target_->getCollider()) {
        return;
    }
    if (collider_->isColliding(*target_->getCollider())) {
        if (stats_ && target_->getStats()) {
            target_->takeDamage(stats_->getDamage());
        }
    }
}

void Enemy::checkState() {
    auto current_state = State::kNormal;
    if (stats_) {
        if (!stats_->isAlive()) {
            current_state = State::kDie;
        } else if (stats_->isInvincible()) {
            current_state = State::kHurt;
        }
    }

    if (state_ != current_state) {
        changeState(current_state);
    }
}

void Enemy::changeState(State state) {
    if (current_anim_) {
        current_anim_->setActive(false);
    }
    switch (state) {
        case State::kNormal:
            current_anim_ = anim_normal_;
            break;
        case State::kHurt:
            current_anim_ = anim_hurt_;
            break;
        case State::kDie:
            current_anim_ = anim_die_;
            break;
    }
    current_anim_->setActive(true);
    state_ = state;
}

void Enemy::remove() {
    if (anim_die_->isFinish()) {
        setNeedRemove(true);
    }
}

}  // namespace sdl3
}  // namespace pyc