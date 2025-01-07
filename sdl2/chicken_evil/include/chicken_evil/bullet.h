#pragma once

#include <chrono>

#include <Eigen/Core>

#include "sdl2/common/camera.h"
#include "sdl2/common/position.h"

namespace pyc {
namespace sdl2 {

class Bullet : public Position {
public:
    explicit Bullet(double angle);

    ~Bullet() = default;

    void on_hit() { is_valid_ = false; }

    bool can_remove() const { return !is_valid_; }

    void on_update(std::chrono::duration<double> delta);

    void on_render(const Camera& camera) const;

private:
    double angle_{};
    Eigen::Vector2d velocity_;
    bool is_valid_{true};
    double speed_ = 800.0;
};

}  // namespace sdl2
}  // namespace pyc
