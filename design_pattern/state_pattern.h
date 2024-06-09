#pragma once

#include <memory>

namespace pyc {

struct Monster;

struct State {
    virtual void update(Monster* monster) = 0;
};

struct Idle : State {
    void update(Monster* monster) override {
        if (monster->seePlayer()) {
            monster->setState(std::make_unique<Chase>());
        }
    }
};

struct Chase : State {
    void update(Monster* monster) override {
        if (monster->canAttack()) {
            monster->setState(std::make_unique<Attack>());
        } else if (!monster->seePlayer()) {
            monster->setState(std::make_unique<Idle>());
        }
    }
};

struct Attack : State {
    void update(Monster* monster) override {
        if (!monster->seePlayer()) {
            monster->setState(std::make_unique<Idle>());
        }
    }
};

struct Monster {
    std::unique_ptr<State> state = std::make_unique<Idle>();

    void update() { state->update(this); }

    void setState(std::unique_ptr<State> new_state) { state = std::move(new_state); }

    virtual bool seePlayer() = 0;

    virtual bool canAttack() = 0;
};

}  // namespace pyc
