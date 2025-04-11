#pragma once

#include "sdl3/common/core/actor.h"
#include "sdl3/common/core/object.h"

namespace pyc {
namespace sdl3 {

class Stats : public Object {
public:
    static Stats* CreateAndSet(Actor* parent, double max_health = 100.0, double max_mana = 100.0,
                               double health_regen = 0.0, double mana_regon = 10.0, double damage = 40.0);

    virtual void update(std::chrono::duration<float> delta) override;

    virtual Actor* getParent() const override;
    virtual void setParent(Object* parent) override;

    double getHealth() const { return health_.value; }
    double getMaxHealth() const { return health_.max_value; }
    double getHealthRegen() const { return health_.regen; }
    double getMana() const { return mana_.value; }
    double getMaxMana() const { return mana_.max_value; }
    double getManaRegen() const { return mana_.regen; }
    double getDamage() const { return damage_; }

    bool isAlive() const { return is_alive_; }

    bool isInvincible() const { return is_invincible_; }

    void takeDamage(double damage);

protected:
    bool canUseMana(double mana_cost) const { return mana_.value >= mana_cost; }
    void useMana(double mana_cost) {
        if (canUseMana(mana_cost)) {
            mana_.value -= mana_cost;
        }
    }

private:
    void invincibleUpdate(std::chrono::duration<float> delta);

protected:
    struct Bar {
        double value{0.0};
        double max_value{0.0};
        double regen{0.0};

        void update(std::chrono::duration<float> delta) {
            value += regen * delta.count();
            if (value > max_value) {
                value = max_value;
            }
        }
    };

    Bar health_{100.0, 100.0, 0.0};
    Bar mana_{100.0, 100.0, 10.0};
    double damage_{40.0};

    bool is_alive_{true};
    bool is_invincible_{false};
    std::chrono::duration<float> invincible_time_{1.5};
    std::chrono::duration<float> invincible_time_counter_{0.0};
};

}  // namespace sdl3
}  // namespace pyc