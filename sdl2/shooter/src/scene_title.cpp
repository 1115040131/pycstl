#include "shooter/scene_title.h"

#include <fmt/base.h>

#include "shooter/game.h"
#include "shooter/scene_main.h"

namespace pyc {
namespace sdl2 {

void SceneTitle::init() {
    bgm_ = Mix_LoadMUS(ASSET("music/06_Battle_in_Space_Intro.ogg"));
    if (!bgm_) {
        fmt::println("Mix_LoadMUS: {}", Mix_GetError());
        return;
    }
    Mix_PlayMusic(bgm_, -1);
}

void SceneTitle::clean() {}

void SceneTitle::update(std::chrono::duration<double> delta) {
    time_ += delta;
    if (time_ > 1s) {
        time_ -= 1s;
    }
}

void SceneTitle::render() {
    game_.renderTextCentered("SDL太空战机", 0.4, game_.title_font());
    if (time_ < 0.5s) {
        game_.renderTextCentered("按 J 键开始游戏", 0.8, game_.text_font());
    }
}

void SceneTitle::handleEvent(SDL_Event* event) {
    if (event->type == SDL_KEYDOWN) {
        if (event->key.keysym.sym == SDLK_j) {
            game_.changeScene(std::make_unique<SceneMain>());
        }
    }
}

}  // namespace sdl2
}  // namespace pyc