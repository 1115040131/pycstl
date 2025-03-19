#pragma once

#include "sdl3/common/actor.h"

namespace pyc {
namespace sdl3 {

class Player : public Actor {
public:
    virtual void init() override;
    virtual void clean() override;

    virtual void handleEvents(SDL_Event& event) override;
    virtual void update(std::chrono::duration<float> delta) override;
    virtual void render() override;

private:
    void keyboardControl();
    void move(std::chrono::duration<float> delta);
    void syncCamera();
};

}  // namespace sdl3
}  // namespace pyc
