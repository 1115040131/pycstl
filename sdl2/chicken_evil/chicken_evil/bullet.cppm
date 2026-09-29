module;

#include <chrono>

#include <Eigen/Core>

export module sdl2.chicken_evil.bullet;

export import sdl2.common.camera;
export import sdl2.common.position;

export namespace pyc::sdl2 {

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

}  // namespace pyc::sdl2
