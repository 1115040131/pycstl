#pragma once

#include <SDL2/SDL_mixer.h>

#include "shooter/scene.h"

namespace pyc {
namespace sdl2 {

class SceneTitle : public Scene {
public:
    virtual ~SceneTitle() = default;

    void init() override;
    void clean() override;

    void update(std::chrono::duration<double> delta) override;
    void render() override;
    void handleEvent(SDL_Event* event) override;

private:
    Mix_Music* bgm_{};
    std::chrono::duration<double> time_{};
};

}  // namespace sdl2
}  // namespace pyc