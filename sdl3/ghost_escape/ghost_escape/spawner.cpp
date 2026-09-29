module;

#include <chrono>

#include <glm/vec2.hpp>

module ghost_escape.spawner;

import ghost_escape.actor;
import ghost_escape.core;
import ghost_escape.enemy;
import ghost_escape.world.effect;

namespace pyc::sdl3 {

void Spawner::update(std::chrono::duration<float> delta) {
    timer_ += delta;
    while (timer_ >= interval_) {
        timer_ -= interval_;
        game_.playSound(Asset("sound/silly-ghost-sound-242342.mp3"));
        for (int i = 0; i < num_; i++) {
            auto position = game_.random(game_.getCurrentScene()->getCameraPosition(),
                                         game_.getCurrentScene()->getCameraPosition() + game_.getScreenSize());

            Effect::CreateAndSet(game_.getCurrentScene().get(), Asset("effect/184_3.png"), position, 1.F,
                                 Enemy::Create(position, target_));
        }
    }
}

}  // namespace pyc::sdl3
