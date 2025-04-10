#include "ghost_escape/spawner.h"

#include "ghost_escape/enemy.h"
#include "sdl3/common/core/scene.h"
#include "sdl3/common/world/effect.h"

namespace pyc {
namespace sdl3 {

void Spawner::update(std::chrono::duration<float> delta) {
    timer_ += delta;
    while (timer_ >= interval_) {
        timer_ -= interval_;
        for (int i = 0; i < num_; i++) {
            auto position = game_.random(game_.getCurrentScene()->getCameraPosition(),
                                         game_.getCurrentScene()->getCameraPosition() + game_.getScreenSize());

            Effect::CreateAndSet(game_.getCurrentScene().get(), ASSET("effect/184_3.png"), position, 1.F,
                                 Enemy::Create(position, target_));
        }
    }
}

}  // namespace sdl3
}  // namespace pyc