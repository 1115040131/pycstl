module;

#include <chrono>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

export module sdl2.shooter.scene_end;

export import sdl2.shooter.scene;

export namespace pyc::sdl2 {

class SceneEnd : public Scene {
public:
    virtual ~SceneEnd() = default;

    void init() override;
    void clean() override;

    void update(std::chrono::duration<double> delta) override;
    void render() override;
    void handleEvent(SDL_Event* event) override;

private:
    void renderPhase1();
    void renderPhase2();

    void removeLastUTF8Char(std::string& str);

private:
    bool is_typing_{true};
    std::string player_name_;
    std::chrono::duration<double> blink_time_{};
    Mix_Music* bgm_{};
};

}  // namespace pyc::sdl2
