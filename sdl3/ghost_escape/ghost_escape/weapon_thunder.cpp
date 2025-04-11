#include "ghost_escape/weapon_thunder.h"

#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

WeaponThunder* WeaponThunder::CreateAndSet(Actor* parent, std::chrono::duration<float> cool_down,
                                           float mana_cost) {
    auto weapon_thunder = std::make_unique<WeaponThunder>();
#ifdef DEBUG_MODE
    weapon_thunder->SET_NAME(WeaponThunder);
#endif
    weapon_thunder->setCoolDown(cool_down);
    weapon_thunder->setManaCost(mana_cost);
    return static_cast<WeaponThunder*>(parent->addChild(std::move(weapon_thunder)));
}

void WeaponThunder::handleEvents(const SDL_Event& event) {
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            if (canAttack()) {
                auto position = game_.getCurrentScene()->screenToWorld(game_.getMousePosition());
                auto spell = Spell::Create(ASSET("effect/Thunderstrike w blur.png"), position, 40.F, 3.F);
                attack(position, std::move(spell));
            }
        }
    }
}

}  // namespace sdl3
}  // namespace pyc