#pragma once

#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

class SceneTitle : public Scene {
public:
    virtual void init() override;
    // virtual void clean() override;

    // virtual void handleEvents(const SDL_Event& event) override;
    virtual void update(std::chrono::duration<float> delta) override;
    virtual void render() override;

private:
    void updateColor(std::chrono::duration<float> delta);

    void renderBackground() const;

private:
    std::chrono::duration<float> timer_{};
    SDL_FColor boundary_color_{0.5, 0.5, 0.5, 1};
};

}  // namespace sdl3
}  // namespace pyc
