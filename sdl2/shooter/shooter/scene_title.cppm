module;

#include <chrono>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

export module sdl2.shooter.scene_title;

export import sdl2.shooter.scene;

export namespace pyc::sdl2 {

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

}  // namespace pyc::sdl2
