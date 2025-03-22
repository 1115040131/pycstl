#pragma once

#include "ghost_escape/player.h"
#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

class SceneMain : public Scene {
public:
    virtual void init() override;
    virtual void clean() override;

    virtual void handleEvents(SDL_Event& event) override;
    virtual void update(std::chrono::duration<float> delta) override;
    virtual void render() override;

private:
    void renderBackground() const;

private:
    Player* player_{};
};

}  // namespace sdl3
}  // namespace pyc
