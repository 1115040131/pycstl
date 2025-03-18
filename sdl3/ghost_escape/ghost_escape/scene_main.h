#pragma once

#include "sdl3/common/scene.h"

namespace pyc {
namespace sdl3 {

class SceneMain : public Scene {
public:
    virtual void init() override;
    virtual void clean() override;

    virtual void handleEvents(SDL_Event& event) override;
    virtual void update(std::chrono::duration<double> delta) override;
    virtual void render() override;
};

}  // namespace sdl3
}  // namespace pyc
