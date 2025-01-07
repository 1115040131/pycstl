#include "sdl2/common/camera.h"

namespace pyc {
namespace sdl2 {

Camera::Camera(SDL_Renderer* render) : renderer(render) {
    timer_shake_.set_one_shot(true);
    timer_shake_.set_on_timeout([this]() {
        is_shaking_ = false;
        reset();
    });
}

void Camera::on_update(std::chrono::duration<double> delta) {
    timer_shake_.on_update(delta);

    if (is_shaking_) {
        position_ = Eigen::Vector2d::Random() * shaking_strength_;
    }
}

void Camera::shake(double strength, std::chrono::duration<double> duration) {
    is_shaking_ = true;
    shaking_strength_ = strength;

    timer_shake_.set_wait_time(duration);
    timer_shake_.restart();
}

void Camera::render_texture(SDL_Texture* texture, const SDL_Rect* rect_src, const SDL_FRect* rect_dst,
                            double angle, const SDL_FPoint* center) const {
    SDL_FRect rect_dst_win = *rect_dst;
    rect_dst_win.x -= position_.x();
    rect_dst_win.y -= position_.y();
    SDL_RenderCopyExF(renderer, texture, rect_src, &rect_dst_win, angle, center, SDL_RendererFlip::SDL_FLIP_NONE);
}

}  // namespace sdl2
}  // namespace pyc
