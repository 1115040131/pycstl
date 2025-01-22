#pragma once

#include <Eigen/Core>
#include <SDL.h>

#include "sdl2/common/position.h"
#include "sdl2/common/timer.h"

namespace pyc {
namespace sdl2 {

class Camera : public Position {
public:
    Camera(SDL_Renderer* render);

    ~Camera() = default;

    void reset() { position_ = Eigen::Vector2d::Zero(); }

    void on_update(std::chrono::duration<double> delta);

    void shake(double strength, std::chrono::duration<double> duration);

    void render_texture(SDL_Texture* texture, const SDL_Rect* rect_src, const SDL_FRect* rect_dst, double angle,
                        const SDL_FPoint* center) const;

private:
    Timer timer_shake_;
    bool is_shaking_{};
    double shaking_strength_{};
    SDL_Renderer* renderer{nullptr};
};

}  // namespace sdl2
}  // namespace pyc
