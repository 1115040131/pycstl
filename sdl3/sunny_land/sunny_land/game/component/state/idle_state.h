#include "sunny_land/game/component/state/player_state.h"

namespace pyc::sunny_land {

class IdleState final : public PlayerState {
    friend class PlayerComponent;

public:
    using PlayerState::PlayerState;

private:
    void enter() override {}
    void exit() override {}
    std::unique_ptr<PlayerState> handleInput(Context&) override;
    std::unique_ptr<PlayerState> update(std::chrono::duration<float>, Context&) override;
};

}  // namespace pyc::sunny_land