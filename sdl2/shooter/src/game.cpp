#include "shooter/game.h"

#include <SDL_image.h>
#include <fmt/base.h>

#include "shooter/scene_main.h"

namespace pyc {
namespace sdl2 {

void Game::run() {
    while (is_running_) {
        SDL_Event event{};
        handleEvent(&event);
        update();
        render();
    }
}

void Game::init() {
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        fmt::println("SDL_Init: {}", SDL_GetError());
        return;
    }

    window_ = SDL_CreateWindow("Shooter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, kWindowWidth,
                               kWindowHeight, SDL_WINDOW_SHOWN);
    if (!window_) {
        fmt::println("SDL_CreateWindow: {}", SDL_GetError());
        return;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        fmt::println("SDL_CreateRenderer: {}", SDL_GetError());
        return;
    }

    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        fmt::println("IMG_Init: {}", IMG_GetError());
        return;
    }

    is_running_ = true;
    changeScene(std::make_unique<SceneMain>());
}

void Game::clean() {
    is_running_ = false;
    if (current_scene_) {
        current_scene_->clean();
    }

    IMG_Quit();

    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Game::changeScene(std::unique_ptr<Scene> scene) {
    if (current_scene_) {
        current_scene_->clean();
    }
    current_scene_ = std::move(scene);
    current_scene_->init();
}

void Game::update() { current_scene_->update(); }

void Game::render() {
    SDL_RenderClear(renderer_);
    current_scene_->render();
    SDL_RenderPresent(renderer_);
}

void Game::handleEvent(SDL_Event* event) {
    while (SDL_PollEvent(event)) {
        if (event->type == SDL_QUIT) {
            is_running_ = false;
        }
        current_scene_->handleEvent(event);
    }
}

}  // namespace sdl2
}  // namespace pyc