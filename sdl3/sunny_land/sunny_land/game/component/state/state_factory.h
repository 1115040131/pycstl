#pragma once

#include <memory>

namespace pyc::sunny_land {

class PlayerComponent;
class PlayerState;

class FallState;
class IdleState;
class JumpState;
class WalkState;

class StateFactory final {
public:
    template <typename T>
    static std::unique_ptr<PlayerState> create(PlayerComponent* player_component);
};

}  // namespace pyc::sunny_land