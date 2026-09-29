module;

#include <chrono>
#include <memory>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <fmt/base.h>

#include "shooter/asset.h"

module sdl2.shooter.scene_title;

import sdl2.shooter.scene;
import sdl2.shooter.scene_main;

namespace pyc::sdl2 {

// object 模块里的 using namespace std::chrono_literals 只对直接 import 它的 TU 生效，
// 本单元没 import object，得自己来一份。
using namespace std::chrono_literals;

void SceneTitle::init() {
    bgm_ = Mix_LoadMUS(ASSET("music/06_Battle_in_Space_Intro.ogg"));
    if (!bgm_) {
        fmt::println("Mix_LoadMUS: {}", Mix_GetError());
        return;
    }
    Mix_PlayMusic(bgm_, -1);
}

void SceneTitle::clean() {
    if (bgm_) {
        Mix_HaltMusic();
        Mix_FreeMusic(bgm_);
    }
}

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

}  // namespace pyc::sdl2
