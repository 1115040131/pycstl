#pragma once

#include "sdl3/common/affiliate/affiliate_bar.h"
#include "sdl3/common/core/object_world.h"
#include "sdl3/common/raw/stats.h"

namespace pyc {
namespace sdl3 {

class Actor : public ObjectWorld {
public:
    virtual void update(std::chrono::duration<float> delta) override;

    const glm::vec2& getVelocity() const { return velocity_; }
    void setVelocity(const glm::vec2& velocity) { velocity_ = velocity; }

    float getMaxSpeed() const { return max_speed_; }
    void setMaxSpeed(float max_speed) { max_speed_ = max_speed; }

    Stats* getStats() const { return stats_; }

    void takeDamage(double damage);
    bool isAlive() const;

protected:
    void move(std::chrono::duration<float> delta);

    void updateHealthBar();

protected:
    glm::vec2 velocity_{};
    float max_speed_{100};

    Stats* stats_{};
    AffiliateBar* health_bar_{};
};

}  // namespace sdl3
}  // namespace pyc
