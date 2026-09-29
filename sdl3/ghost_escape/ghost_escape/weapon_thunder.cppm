module;

#include <chrono>

#include <SDL3/SDL.h>

export module ghost_escape.weapon_thunder;

export import ghost_escape.raw.weapon;
import ghost_escape.screen.hud_skill;

export namespace pyc::sdl3 {

class WeaponThunder : public Weapon {
public:
    static WeaponThunder* CreateAndSet(Actor* parent, std::chrono::duration<float> cool_down, float mana_cost);

    virtual void update(std::chrono::duration<float> delta) override;

    virtual bool handleEvents(const SDL_Event& event) override;

private:
    HUDSkill* hud_skill_{};
};

}  // namespace pyc::sdl3
