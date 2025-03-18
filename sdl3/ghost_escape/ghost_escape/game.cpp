#include "ghost_escape/game.h"

#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <fmt/base.h>

namespace pyc {
namespace sdl3 {

void Game::init(std::string_view title, int width, int height) {
    title_ = title;
    screen_size_ = glm::vec2(width, height);

    // SDL3 初始化
    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        fmt::println("SDL_Init: {}", SDL_GetError());
        return;
    }

    // SDL3_Mixer 初始化
    if (Mix_Init(MIX_INIT_MP3 | MIX_INIT_OGG) != (MIX_INIT_MP3 | MIX_INIT_OGG)) {
        fmt::println("Mix_Init: {}", SDL_GetError());
        return;
    }
    if (!Mix_OpenAudio(0, nullptr)) {
        fmt::println("Mix_OpenAudio: {}", SDL_GetError());
        return;
    }
    Mix_AllocateChannels(16);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 4);
    Mix_Volume(-1, MIX_MAX_VOLUME / 4);

    // SDL3_TTF 初始化
    if (!TTF_Init()) {
        fmt::println("TTF_Init: {}", SDL_GetError());
        return;
    }

    // 创建窗口与渲染器
    SDL_CreateWindowAndRenderer(title.data(), width, height, SDL_WINDOW_RESIZABLE, &window_, &renderer_);
    if (!window_ || !renderer_) {
        fmt::println("SDL_CreateWindowAndRenderer: {}", SDL_GetError());
        return;
    }

    // 设置窗口分辨率
    SDL_SetRenderLogicalPresentation(renderer_, width, height, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    is_running_ = true;
}

void Game::clean() {}

void Game::run() {
    while (is_running_) {
        handleEvents();
        update(std::chrono::duration<double>(0));
        render();
    }
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                is_running_ = false;
                break;
            default:
                break;
        }
    }
}

void Game::update(std::chrono::duration<double> delta) { (void)delta; }

void Game::render() {}

}  // namespace sdl3
}  // namespace pyc