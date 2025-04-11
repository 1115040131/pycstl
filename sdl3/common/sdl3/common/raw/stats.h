#pragma once

#include "sdl3/common/core/actor.h"
#include "sdl3/common/core/object.h"

namespace pyc {
namespace sdl3 {

class Stats : public Object {
public:
    static Stats* CreateAndSet(Actor* parent, float max_health = 100.0, float max_mana = 100.0,
                               float health_regen = 0.0, float mana_regon = 10.0, float damage = 40.0);

    virtual void update(std::chrono::duration<float> delta) override;

    virtual Actor* getParent() const override;
    virtual void setParent(Object* parent) override;

    float getHealth() const { return health_.value; }
    float getMaxHealth() const { return health_.max_value; }
    float getHealthRegen() const { return health_.regen; }
    float getMana() const { return mana_.value; }
    float getMaxMana() const { return mana_.max_value; }
    float getManaRegen() const { return mana_.regen; }
    float getDamage() const { return damage_; }

    bool isAlive() const { return is_alive_; }

    bool isInvincible() const { return is_invincible_; }

    void takeDamage(float damage);

    bool canUseMana(float mana_cost) const { return mana_.value >= mana_cost; }

    void useMana(float mana_cost) {
        if (canUseMana(mana_cost)) {
            mana_.value -= mana_cost;
        }
    }

private:
    void invincibleUpdate(std::chrono::duration<float> delta);

protected:
    struct Bar {
        float value{0.0};
        float max_value{0.0};
        float regen{0.0};

        void update(std::chrono::duration<float> delta) {
            value += regen * delta.count();
            if (value > max_value) {
                value = max_value;
            }
        }
    };

    Bar health_{100.0, 100.0, 0.0};
    Bar mana_{100.0, 100.0, 10.0};
    float damage_{40.0};

    bool is_alive_{true};
    bool is_invincible_{false};
    std::chrono::duration<float> invincible_time_{1.5};
    std::chrono::duration<float> invincible_time_counter_{0.0};
};

}  // namespace sdl3
}  // namespace pyc