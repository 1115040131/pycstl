#pragma once

#include <vector>

#include <Eigen/Core>
#include <SDL.h>

#include "sdl2/common/atlas.h"
#include "sdl2/common/camera.h"
#include "sdl2/common/position.h"
#include "sdl2/common/timer.h"

namespace pyc {
namespace sdl2 {

class Animation : public Position {
public:
    Animation();

    ~Animation() = default;

    void reset() {
        timer_.restart();
        frame_idx_ = 0;
    }

    void set_rotation(double angle) { angle_ = angle; }

    void set_center(const SDL_FPoint& center) { center_ = center; }

    void set_loop(bool is_loop) { is_loop_ = is_loop; }

    void set_interval(std::chrono::duration<double> interval) { timer_.set_wait_time(interval); }

    void set_on_finished(std::function<void()> on_finished) { on_finished_ = on_finished; }

    void add_frame(SDL_Texture* texture, int num);

    void add_frame(const Atlas& atlas);

    void on_update(std::chrono::duration<double> delta) { timer_.on_update(delta); }

    void on_render(const Camera& camera) const;

private:
    struct Frame {
        SDL_Rect rect_src{};
        SDL_Texture* texture{nullptr};
    };

private:
    double angle_{};
    SDL_FPoint center_{};

    Timer timer_;
    bool is_loop_{true};
    size_t frame_idx_{};
    std::vector<Frame> frames_;
    std::function<void()> on_finished_{nullptr};
};

}  // namespace sdl2
}  // namespace pyc
