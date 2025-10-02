#pragma once

#include <chrono>

#include "sunny_land/game/component/state/player_state.h"

namespace pyc::sunny_land {

class HurtState final : public PlayerState {
    friend class PlayerComponent;

public:
    using PlayerState::PlayerState;

private:
    void enter() override;
    void exit() override {}
    std::unique_ptr<PlayerState> handleInput(Context&) override;
    std::unique_ptr<PlayerState> update(std::chrono::duration<float>, Context&) override;

private:
    std::chrono::duration<float> stunned_timer_{};  ///< @brief 硬直计时器
};

}  // namespace pyc::sunny_land