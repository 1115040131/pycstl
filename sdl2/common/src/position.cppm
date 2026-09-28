module;

#include <Eigen/Core>

export module sdl2.common.position;

export namespace pyc::sdl2 {

class Position {
public:
    void set_position(const Eigen::Vector2d& position) { position_ = position; }

    const Eigen::Vector2d& position() const { return position_; }

protected:
    Eigen::Vector2d position_;
};

}  // namespace pyc::sdl2
