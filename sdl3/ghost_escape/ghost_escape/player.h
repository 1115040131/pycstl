#pragma once

#include "ghost_escape/weapon_thunder.h"
#include "sdl3/common/affiliate/sprite_anim.h"
#include "sdl3/common/core/actor.h"
#include "sdl3/common/world/effect.h"

namespace pyc {
namespace sdl3 {

class Player : public Actor {
public:
    virtual void init() override;
    virtual void clean() override;

    virtual void handleEvents(const SDL_Event& event) override;
    virtual void update(std::chrono::duration<float> delta) override;
    virtual void render() override;

    virtual void takeDamage(double damage) override;

private:
    void keyboardControl();
    void syncCamera();
    void checkState();
    void checkIsDead();

private:
    SpriteAnim* anim_idle_{};
    SpriteAnim* anim_move_{};
    std::unique_ptr<Effect> effect_{};
    WeaponThunder* weapon_thunder_{};
    bool is_moving_{};
};

}  // namespace sdl3
}  // namespace pyc
