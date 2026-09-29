module;

#include <chrono>

export module sdl2.chicken_evil.chicken;

export import sdl2.common.animation;
export import sdl2.common.camera;
export import sdl2.common.position;

export namespace pyc::sdl2 {

class Chicken : public Position {
public:
    Chicken();

    ~Chicken() = default;

    void on_update(std::chrono::duration<double> delta);

    void on_render(const Camera& camera) const;

    void on_hurt();

    void make_invalid() { is_alive_ = false; }

    bool check_alive() const { return is_alive_; }

    bool can_remove() { return !is_valid_; }

protected:
    double speed_ = 10.0;
    Animation animation_run_;

private:
    Animation animation_explosion_;
    Animation* current_animation_ = nullptr;

    bool is_alive_ = true;
    bool is_valid_ = true;
};

class ChickenFast : public Chicken {
public:
    ChickenFast();

    ~ChickenFast() = default;
};

class ChickenMedium : public Chicken {
public:
    ChickenMedium();

    ~ChickenMedium() = default;
};

class ChickenSlow : public Chicken {
public:
    ChickenSlow();

    ~ChickenSlow() = default;
};

}  // namespace pyc::sdl2
