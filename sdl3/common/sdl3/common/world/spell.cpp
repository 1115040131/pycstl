#include "sdl3/common/world/spell.h"

#include "sdl3/common/core/actor.h"
#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

Spell* Spell::CreateAndSet(Object* parent, const std::string& file_path, const glm::vec2& position, float damage,
                           float scale, Anchor anchor) {
    auto spell = std::make_unique<Spell>();
    spell->init();
#ifdef DEBUG_MODE
    spell->SET_NAME(Spell);
#endif
    spell->damage_ = damage;
    spell->sprite_anim_ = SpriteAnim::CreateAndSet(spell.get(), file_path, scale, 10.F, false, anchor);
    spell->collider_ =
        Collider::CreateAndSet(spell.get(), spell->sprite_anim_->getSize(), Collider::Type::kCircle, anchor);
    spell->setPosition(position);
    return static_cast<Spell*>(parent->addChild(std::move(spell)));
}

void Spell::update(std::chrono::duration<float> delta) {
    ObjectWorld::update(delta);
    if (sprite_anim_) {
        if (sprite_anim_->isFinish()) {
            this->setNeedRemove(true);
        } else {
            attack();
        }
    }
}

void Spell::attack() {
    for (const auto& child : game_.getCurrentScene()->getChildrenWorld()) {
        if (child->getType() != Object::Type::kEnemy) {
            continue;
        }
        if (collider_ && child->getCollider() && collider_->isColliding(*child->getCollider())) {
            auto enemy = static_cast<Actor*>(child.get());
            enemy->takeDamage(damage_);
        }
    }
}

}  // namespace sdl3
}  // namespace pyc