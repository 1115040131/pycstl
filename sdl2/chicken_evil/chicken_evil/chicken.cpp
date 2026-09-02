#include "chicken_evil/chicken.h"

#include <random>

#include <SDL2/SDL_mixer.h>

#include "chicken_evil/resource_mgr.h"

namespace pyc {
namespace sdl2 {

using namespace std::chrono_literals;

class Random {
public:
    Random() { e_.seed(time(0)); }

    int operator()() { return e_(); }

private:
    std::default_random_engine e_;
};

inline Random random;

Chicken::Chicken() {
    animation_run_.set_loop(true);
    animation_run_.set_interval(0.1s);
    // animation_run_.add_frame 在子类中添加

    animation_explosion_.set_loop(false);
    animation_explosion_.set_interval(0.08s);
    animation_explosion_.add_frame(ResourceMgr::GetInstance().AtlasExplosion());
    animation_explosion_.set_on_finished([&]() { is_valid_ = false; });

    position_.x() = 40.0 + random() % 1200;
    position_.y() = -50.0;
}

void Chicken::on_update(std::chrono::duration<double> delta) {
    if (is_alive_) {
        position_.y() += speed_ * delta.count();
    }

    current_animation_ = is_alive_ ? &animation_run_ : &animation_explosion_;
    current_animation_->set_position(position_);
    current_animation_->on_update(delta);
}

void Chicken::on_render(const Camera& camera) const { current_animation_->on_render(camera); }

void Chicken::on_hurt() {
    is_alive_ = false;

    Mix_PlayChannel(-1, ResourceMgr::GetInstance().SoundExplosion(), 0);
}

ChickenFast::ChickenFast() : Chicken() {
    animation_run_.add_frame(ResourceMgr::GetInstance().AtlasChickenFast());
    speed_ = 80.0;
}

ChickenMedium::ChickenMedium() : Chicken() {
    animation_run_.add_frame(ResourceMgr::GetInstance().AtlasChickenMedium());
    speed_ = 50.0;
}

ChickenSlow::ChickenSlow() : Chicken() {
    animation_run_.add_frame(ResourceMgr::GetInstance().AtlasChickenSlow());
    speed_ = 30.0;
}

}  // namespace sdl2
}  // namespace pyc
