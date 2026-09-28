module;

#include <chrono>
#include <memory>

export module sunny_land.game.player:dead_state;

export import :player_state;

export namespace pyc::sunny_land {

class DeadState final : public PlayerState {
    friend class PlayerComponent;

public:
    using PlayerState::PlayerState;

private:
    void enter() override;
    void exit() override {}
    std::unique_ptr<PlayerState> handleInput(Context&) override;
    std::unique_ptr<PlayerState> update(std::chrono::duration<float>, Context&) override;
};

}  // namespace pyc::sunny_land
