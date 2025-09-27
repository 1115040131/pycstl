#include "sunny_land/game/component/state/state_factory.h"

#include "sunny_land/game/component/state/fall_state.h"
#include "sunny_land/game/component/state/idle_state.h"
#include "sunny_land/game/component/state/jump_state.h"
#include "sunny_land/game/component/state/walk_state.h"

namespace pyc::sunny_land {

template <typename T>
std::unique_ptr<PlayerState> StateFactory::create(PlayerComponent* player_component) {
    return std::make_unique<T>(player_component);
}

template std::unique_ptr<PlayerState> StateFactory::create<FallState>(PlayerComponent* player_component);
template std::unique_ptr<PlayerState> StateFactory::create<IdleState>(PlayerComponent* player_component);
template std::unique_ptr<PlayerState> StateFactory::create<JumpState>(PlayerComponent* player_component);
template std::unique_ptr<PlayerState> StateFactory::create<WalkState>(PlayerComponent* player_component);

}  // namespace pyc::sunny_land