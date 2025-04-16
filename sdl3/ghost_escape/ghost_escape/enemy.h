#pragma once

#include "sdl3/common/affiliate/sprite_anim.h"
#include "sdl3/common/core/actor.h"

namespace pyc {
namespace sdl3 {

class Enemy : public Actor {
public:
    static std::unique_ptr<Enemy> Create(const glm::vec2& position, Actor* target);

    virtual void init() override;

    virtual void update(std::chrono::duration<float> delta) override;

    void setTarget(Actor* target) { target_ = target; }

private:
    enum class State {
        kNormal,
        kHurt,
        kDie,
    };

    void aimTarget();
    void attack();
    void checkState();
    void changeState(State state);

    void remove();

private:
    State state_{State::kNormal};

    Actor* target_{};

    SpriteAnim* anim_normal_{};
    SpriteAnim* anim_hurt_{};
    SpriteAnim* anim_die_{};

    SpriteAnim* current_anim_{};

    int score_{10};
};

}  // namespace sdl3
}  // namespace pyc