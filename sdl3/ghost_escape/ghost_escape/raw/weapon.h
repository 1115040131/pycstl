#pragma once

#include <chrono>

#include "ghost_escape/core/actor.h"
#include "ghost_escape/core/object.h"
#include "ghost_escape/world/spell.h"

namespace pyc {
namespace sdl3 {

class Weapon : public Object {
public:
    virtual void update(std::chrono::duration<float> delta) override;

    virtual Actor* getParent() const override;
    virtual void setParent(Object* parent) override;

    void setCoolDown(std::chrono::duration<float> cool_down) { cool_down_ = cool_down; }
    void setManaCost(float mana_cost) { mana_cost_ = mana_cost; }

    float getCoolDownPercent() const;

    bool canAttack() const;

    void attack(const glm::vec2& position, std::unique_ptr<Spell> spell);

protected:
    std::chrono::duration<float> timer_{std::chrono::duration<float>::max()};
    std::chrono::duration<float> cool_down_{1.F};
    float mana_cost_{};
};

}  // namespace sdl3
}  // namespace pyc