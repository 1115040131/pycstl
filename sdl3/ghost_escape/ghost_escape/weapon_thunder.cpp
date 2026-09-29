module;

#include <chrono>
#include <memory>
#include <utility>

#include <SDL3/SDL.h>
#include <glm/vec2.hpp>

#include "ghost_escape/core/asset.h"
#include "ghost_escape/core/set_name.h"

module ghost_escape.weapon_thunder;

import ghost_escape.core;
import ghost_escape.raw.weapon;
import ghost_escape.screen.hud_skill;

namespace pyc::sdl3 {

WeaponThunder* WeaponThunder::CreateAndSet(Actor* parent, std::chrono::duration<float> cool_down,
                                           float mana_cost) {
    auto weapon_thunder = std::make_unique<WeaponThunder>();
    weapon_thunder->init();
#ifdef DEBUG_MODE
    weapon_thunder->SET_NAME(WeaponThunder);
#endif
    weapon_thunder->setCoolDown(cool_down);
    weapon_thunder->setManaCost(mana_cost);
    const auto& game = Game::GetInstance();
    weapon_thunder->hud_skill_ =
        HUDSkill::CreateAndSet(game.getCurrentScene().get(), ASSET("UI/Electric-Icon.png"),
                               glm::vec2(game.getScreenSize().x - 300, 30), 0.14f);
    return static_cast<WeaponThunder*>(parent->addChild(std::move(weapon_thunder)));
}

void WeaponThunder::update(std::chrono::duration<float> delta) {
    Weapon::update(delta);
    if (hud_skill_) {
        hud_skill_->setPercent(getCoolDownPercent());
    }
}

bool WeaponThunder::handleEvents(const SDL_Event& event) {
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            if (canAttack()) {
                game_.playSound(ASSET("sound/big-thunder.mp3"));
                auto position = game_.getCurrentScene()->screenToWorld(game_.getMousePosition());
                auto spell = Spell::Create(ASSET("effect/Thunderstrike w blur.png"), position, 40.F, 3.F);
                attack(position, std::move(spell));
                return true;
            }
        }
    }
    return false;
}

}  // namespace pyc::sdl3
