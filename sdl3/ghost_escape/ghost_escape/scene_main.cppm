module;

#include <chrono>
#include <string_view>

export module ghost_escape.scene_main;

export import ghost_escape.core;
import ghost_escape.hud_stats;
import ghost_escape.player;
import ghost_escape.raw.timer;
import ghost_escape.screen.hud_button;
import ghost_escape.screen.hud_text;
import ghost_escape.screen.ui_mouse;
import ghost_escape.spawner;

export namespace pyc::sdl3 {

class SceneMain : public Scene {
public:
    virtual void init() override;
    virtual void clean() override;

    virtual void update(std::chrono::duration<float> delta) override;
    virtual void render() override;

    virtual void saveData(std::string_view file_path) const override;

private:
    float checkSlowDown();

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

}  // namespace pyc::sdl3
