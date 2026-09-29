module;

#include <chrono>
#include <memory>
#include <string>
#include <utility>

#include <glm/glm.hpp>

module ghost_escape.world.effect;

import ghost_escape.affiliate.sprite_anim;
import ghost_escape.core;

namespace pyc::sdl3 {

Effect* Effect::CreateAndSet(Object* parent, const std::string& file_path, const glm::vec2& position, float scale,
                             std::unique_ptr<ObjectWorld> next) {
    auto effect = std::make_unique<Effect>();
    effect->init();
    SetDebugName(effect.get());
    effect->sprite_ = SpriteAnim::CreateAndSet(effect.get(), file_path, scale, 10.F, false);
    effect->setPosition(position);
    effect->setNext(std::move(next));
    return static_cast<Effect*>(parent->addChild(std::move(effect)));
}

void Effect::clean() {
    if (next_) {
        next_->clean();
    }
}

void Effect::update(std::chrono::duration<float> delta) {
    ObjectWorld::update(delta);
    checkFinish();
}

void Effect::checkFinish() {
    if (sprite_ && sprite_->isFinish()) {
        need_remove_ = true;
        if (next_) {
            game_.getCurrentScene()->safeAddChild(std::move(next_));
        }
    }
}

}  // namespace pyc::sdl3
