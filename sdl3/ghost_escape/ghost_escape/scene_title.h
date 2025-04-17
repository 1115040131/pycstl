#pragma once

#include "sdl3/common/core/scene.h"
#include "sdl3/common/screen/hud_button.h"

namespace pyc {
namespace sdl3 {

class SceneTitle : public Scene {
public:
    virtual void init() override;

    virtual void update(std::chrono::duration<float> delta) override;
    virtual void render() override;

private:
    void updateColor(std::chrono::duration<float> delta);

    void checkButtonStart();
    void checkButtonQuit();

    void renderBackground() const;

private:
    std::chrono::duration<float> timer_{};
    SDL_FColor boundary_color_{0.5, 0.5, 0.5, 1};

    HUDButton* button_start_{};
    HUDButton* button_credits_{};
    HUDButton* button_quit_{};
};

}  // namespace sdl3
}  // namespace pyc
