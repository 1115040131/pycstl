#pragma once

#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

class Player;
class Spawner;
class UIMouse;
class HUDStatus;
class HUDText;
class HUDButton;
class Timer;

class SceneMain : public Scene {
public:
    virtual void init() override;
    virtual void clean() override;

    virtual void update(std::chrono::duration<float> delta) override;
    virtual void render() override;

private:
    void updateScore();

    void checkButtonPause();
    void checkButtonRestart();
    void checkButtonBack();
    void checkEndTimer();

    void renderBackground() const;

private:
    Player* player_{};
    Spawner* spawner_{};
    UIMouse* ui_mouse_{};
    HUDStatus* hud_stats_{};
    HUDText* hud_text_score_{};
    HUDButton* button_pause_{};
    HUDButton* button_restart_{};
    HUDButton* button_back_{};
    Timer* end_timer_{};
};

}  // namespace sdl3
}  // namespace pyc
