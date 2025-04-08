#include "sdl3/common/world/effect.h"

#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

std::unique_ptr<Effect> Effect::Create(const std::string& file_path, const glm::vec2& position, float scale,
                                 std::unique_ptr<ObjectWorld> next) {
    auto effect = std::make_unique<Effect>();
    effect->init();
    effect->sprite_ = SpriteAnim::CreateAndSet(effect.get(), file_path, scale, 10.F, false);
    effect->setPosition(position);
    effect->setNext(std::move(next));
    return effect;
}

Effect* Effect::CreateAndSet(Object* parent, const std::string& file_path, const glm::vec2& position, float scale,
                             std::unique_ptr<ObjectWorld> next) {
    auto effect = std::make_unique<Effect>();
    effect->init();
    effect->sprite_ = SpriteAnim::CreateAndSet(effect.get(), file_path, scale, 10.F, false);
    effect->setPosition(position);
    effect->setNext(std::move(next));
    return static_cast<Effect*>(parent->addChild(std::move(effect)));
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

}  // namespace sdl3
}  // namespace pyc