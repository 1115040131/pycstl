#pragma once

#include <Eigen/Core>

namespace pyc {
namespace sdl2 {

class Position {
public:
    void set_position(const Eigen::Vector2d& position) { position_ = position; }

    const Eigen::Vector2d& position() const { return position_; }

protected:
    Eigen::Vector2d position_;
};

}  // namespace sdl2
}  // namespace pyc
