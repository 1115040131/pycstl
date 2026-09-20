module;

#include <memory>

export module design_pattern.state_pattern;

export namespace pyc {

struct Monster;

struct State {
    virtual ~State() = default;
    virtual void update(Monster* monster) = 0;
};

struct Idle : State {
    void update(Monster* monster) override;
};

struct Chase : State {
    void update(Monster* monster) override;
};

struct Attack : State {
    void update(Monster* monster) override;
};

struct Monster {
    std::unique_ptr<State> state = std::make_unique<Idle>();

    void update() { state->update(this); }

    void setState(std::unique_ptr<State> new_state) { state = std::move(new_state); }

    virtual bool seePlayer() = 0;

    virtual bool canAttack() = 0;
};

// 状态互转天然成环(Idle→Chase→Attack→Idle), 定义只能放在 Monster 完整之后
void Idle::update(Monster* monster) {
    if (monster->seePlayer()) {
        monster->setState(std::make_unique<Chase>());
    }
}

void Chase::update(Monster* monster) {
    if (monster->canAttack()) {
        monster->setState(std::make_unique<Attack>());
    } else if (!monster->seePlayer()) {
        monster->setState(std::make_unique<Idle>());
    }
}

void Attack::update(Monster* monster) {
    if (!monster->seePlayer()) {
        monster->setState(std::make_unique<Idle>());
    }
}

}  // namespace pyc
