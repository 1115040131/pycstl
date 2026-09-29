module;

#include <memory>

module sunny_land.game.player;

namespace pyc::sunny_land {

template <typename T>
std::unique_ptr<PlayerState> StateFactory::create(PlayerComponent* player_component) {
    return std::make_unique<T>(player_component);
}

template std::unique_ptr<PlayerState> StateFactory::create<ClimbState>(PlayerComponent* player_component);
template std::unique_ptr<PlayerState> StateFactory::create<DeadState>(PlayerComponent* player_component);
template std::unique_ptr<PlayerState> StateFactory::create<FallState>(PlayerComponent* player_component);
template std::unique_ptr<PlayerState> StateFactory::create<HurtState>(PlayerComponent* player_component);
template std::unique_ptr<PlayerState> StateFactory::create<IdleState>(PlayerComponent* player_component);
template std::unique_ptr<PlayerState> StateFactory::create<JumpState>(PlayerComponent* player_component);
template std::unique_ptr<PlayerState> StateFactory::create<WalkState>(PlayerComponent* player_component);

}  // namespace pyc::sunny_land
