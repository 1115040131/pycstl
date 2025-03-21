#pragma once

#include "sdl3/common/core/object_world.h"

namespace pyc {
namespace sdl3 {

class Actor : public ObjectWorld {
public:
    const glm::vec2& getVelocity() const { return velocity_; }
    void setVelocity(const glm::vec2& velocity) { velocity_ = velocity; }

    float getMaxSpeed() const { return max_speed_; }
    void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }

protected:
    glm::vec2 velocity_{};
    float max_speed_{100};
};

}  // namespace sdl3
}  // namespace pyc
