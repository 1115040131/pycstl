#include "shooter/scene_end.h"

#include <fmt/format.h>

#include "shooter/game.h"
#include "shooter/scene_main.h"

namespace pyc {
namespace sdl2 {

void SceneEnd::init() {
    if (!SDL_IsTextInputActive()) {
        SDL_StartTextInput();
    }
    if (!SDL_IsTextInputActive()) {
        fmt::println("Failed to start text input: {}", SDL_GetError());
    }
}

void SceneEnd::clean() {}

void SceneEnd::update(std::chrono::duration<double> delta) {
    blink_time_ += delta;
    if (blink_time_ > 1.0s) {
        blink_time_ -= 1.0s;
    }
}

void SceneEnd::render() {
    if (is_typing_) {
        renderPhase1();
    } else {
        renderPhase2();
    }
}

void SceneEnd::handleEvent(SDL_Event* event) {
    if (is_typing_) {
        if (event->type == SDL_TEXTINPUT) {
            player_name_ += event->text.text;
        } else if (event->type == SDL_KEYDOWN) {
            if (event->key.keysym.scancode == SDL_SCANCODE_RETURN) {
                is_typing_ = false;
                SDL_StopTextInput();
                if (player_name_.empty()) {
                    player_name_ = "无名氏";
                }
                game_.insertLeaderBoard(player_name_, game_.finalScore());
            } else if (event->key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
                if (!player_name_.empty()) {
                    removeLastUTF8Char(player_name_);
                }
            }
        }
    } else {
        if (event->type == SDL_KEYDOWN) {
            if (event->key.keysym.scancode == SDL_SCANCODE_J) {
                game_.changeScene(std::make_unique<SceneMain>());
            }
        }
    }
}

void SceneEnd::renderPhase1() {
    game_.renderTextCentered(fmt::format("你的得分是: {}", game_.finalScore()), 0.1, game_.text_font());
    game_.renderTextCentered("Game Over", 0.4, game_.title_font());
    game_.renderTextCentered("请输入你的名字, 按回车确认: ", 0.6, game_.text_font());

    if (!player_name_.empty()) {
        auto point = game_.renderTextCentered(player_name_, 0.8, game_.text_font());
        if (blink_time_ < 0.5s) {
            game_.renderText("_", point, game_.text_font());
        }
    } else {
        if (blink_time_ < 0.5s) {
            game_.renderTextCentered("_", 0.8, game_.text_font());
        }
    }
}

void SceneEnd::renderPhase2() {
    game_.renderTextCentered("得分榜", 0.1, game_.title_font());
    int i = 1;
    auto y = static_cast<int>(0.2 * Game::kWindowHeight);
    for (const auto& item : game_.leaderBoard()) {
        game_.renderText(fmt::format("{}. {}", i, item.second), {100, y}, game_.text_font());
        game_.renderText(std::to_string(item.first), {100, y}, game_.text_font(), false);
        i++;
        y += 45;
    }
    if (blink_time_ < 0.5s) {
        game_.renderTextCentered("按 J 重新开始游戏", 0.85, game_.text_font());
    }
}

void SceneEnd::removeLastUTF8Char(std::string& str) {
    if (str.empty()) {
        return;
    }
    auto last_char = str.back();
    if (last_char & 0b10000000) {
        str.pop_back();
        while ((str.back() & 0b11000000) != 0b11000000) {
            str.pop_back();
        }
    }
    str.pop_back();
}

}  // namespace sdl2
}  // namespace pyc