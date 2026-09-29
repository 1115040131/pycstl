module;

#include <chrono>

#include <Eigen/Core>
#include <SDL2/SDL.h>

export module sdl2.common.camera;

export import sdl2.common.position;

import sdl2.common.timer;

export namespace pyc::sdl2 {

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

}  // namespace pyc::sdl2
