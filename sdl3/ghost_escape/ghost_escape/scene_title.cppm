module;

#include <chrono>
#include <string_view>

#include <SDL3/SDL.h>

export module ghost_escape.scene_title;

export import ghost_escape.core;
import ghost_escape.screen.hud_text;
import ghost_escape.screen.hud_button;
import ghost_escape.screen.ui_mouse;

export namespace pyc::sdl3 {

class SceneTitle : public Scene {
public:
    virtual void init() override;

    virtual bool handleEvents(const SDL_Event& event) override;
    virtual void update(std::chrono::duration<float> delta) override;
    virtual void render() override;

    virtual void loadData(std::string_view file_path) const override;

private:
    void updateColor(std::chrono::duration<float> delta);

    void checkButtonStart();
    void checkButtonCredits();
    void checkButtonQuit();

    void renderBackground() const;

private:
    std::chrono::duration<float> timer_{};
    SDL_FColor boundary_color_{0.5, 0.5, 0.5, 1};

    HUDText* credits_text_{};

    HUDButton* button_start_{};
    HUDButton* button_credits_{};
    HUDButton* button_quit_{};

    UIMouse* ui_mouse_{};
};

}  // namespace pyc::sdl3
