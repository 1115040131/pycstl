module;

#include <chrono>
#include <cmath>

#include <SDL2/SDL.h>

module sdl2.chicken_evil.bullet;

import sdl2.chicken_evil.resource_mgr;
import sdl2.common.camera;

namespace pyc::sdl2 {

Bullet::Bullet(double angle) : angle_(angle) {
    double rad = angle * M_PI / 180;
    velocity_.x() = std::cos(rad) * speed_;
    velocity_.y() = std::sin(rad) * speed_;
}

void Bullet::on_update(std::chrono::duration<double> delta) {
    position_ += velocity_ * delta.count();
    if (position_.x() < 0 || position_.x() > 1280 || position_.y() < 0 || position_.y() > 720) {
        is_valid_ = false;
    }
}

void Bullet::on_render(const Camera& camera) const {
    SDL_FRect bullet{static_cast<float>(position_.x() - 4), static_cast<float>(position_.y() - 4), 8, 4};
    camera.render_texture(ResourceMgr::GetInstance().TextureBullet(), nullptr, &bullet, angle_, nullptr);
}

}  // namespace pyc::sdl2
