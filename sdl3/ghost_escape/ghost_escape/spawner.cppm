module;

#include <chrono>

export module ghost_escape.spawner;

export import ghost_escape.core;
export import ghost_escape.actor;

export namespace pyc::sdl3 {

class Spawner : public Object {
public:
    virtual void update(std::chrono::duration<float> delta) override;

    void setTarget(Actor* target) { target_ = target; }

protected:
    int num_ = 20;
    std::chrono::duration<float> timer_{};
    std::chrono::duration<float> interval_{3.0f};

    Actor* target_{};
};

}  // namespace pyc::sdl3
