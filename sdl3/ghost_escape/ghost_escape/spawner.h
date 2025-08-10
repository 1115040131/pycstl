#pragma once

#include <chrono>

#include "ghost_escape/core/actor.h"
#include "ghost_escape/core/object.h"

namespace pyc {
namespace sdl3 {

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

}  // namespace sdl3
}  // namespace pyc