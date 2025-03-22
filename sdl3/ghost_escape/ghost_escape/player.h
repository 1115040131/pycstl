#pragma once

#include "sdl3/common/affiliate/sprite_anim.h"
#include "sdl3/common/core/actor.h"

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
    void syncCamera();
    void checkState();

private:
    SpriteAnim* anim_idle_{};
    SpriteAnim* anim_move_{};
    bool is_moving_{};
};

}  // namespace sdl3
}  // namespace pyc
