#pragma once

#include "sdl3/common/raw/weapon.h"
#include "sdl3/common/screen/hud_skill.h"

namespace pyc {
namespace sdl3 {

class WeaponThunder : public Weapon {
public:
    static WeaponThunder* CreateAndSet(Actor* parent, std::chrono::duration<float> cool_down, float mana_cost);

    virtual void update(std::chrono::duration<float> delta) override;

    virtual void handleEvents(const SDL_Event& event) override;

private:
    HUDSkill* hud_skill_{};
};

}  // namespace sdl3
}  // namespace pyc