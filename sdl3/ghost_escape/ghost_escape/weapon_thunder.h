#pragma once

#include "sdl3/common/raw/weapon.h"

namespace pyc {
namespace sdl3 {

class WeaponThunder : public Weapon {
public:
    static WeaponThunder* CreateAndSet(Actor* parent, std::chrono::duration<float> cool_down, float mana_cost);

    virtual void handleEvents(const SDL_Event& event) override;
};

}  // namespace sdl3
}  // namespace pyc