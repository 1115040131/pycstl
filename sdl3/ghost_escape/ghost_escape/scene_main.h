#pragma once

#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

class Player;
class Spawner;
class UIMouse;

class SceneMain : public Scene {
public:
    virtual void init() override;
    virtual void clean() override;

    virtual void handleEvents(const SDL_Event& event) override;
    virtual void update(std::chrono::duration<float> delta) override;
    virtual void render() override;

private:
    void renderBackground() const;

private:
    Player* player_{};
    Spawner* spawner_{};
    UIMouse* ui_mouse_{};
};

}  // namespace sdl3
}  // namespace pyc
