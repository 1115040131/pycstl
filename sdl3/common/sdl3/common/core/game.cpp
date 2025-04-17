#include "sdl3/common/core/game.h"

#include <chrono>
#include <thread>

#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

Game::Game() = default;

Game::~Game() = default;

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

    // 创建字体引擎
    ttf_engine_ = TTF_CreateRendererTextEngine(renderer_);

    // 创建 AssetStore
    asset_store_ = std::make_unique<AssetStore>(renderer_);

    is_running_ = true;
}

void Game::clean() {
    if (current_scene_) {
        current_scene_->clean();
    }

    asset_store_.reset();

    // 释放渲染器和窗口
    if (ttf_engine_) {
        TTF_DestroyRendererTextEngine(ttf_engine_);
    }
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_) {
        SDL_DestroyWindow(window_);
    }
    // 退出 Mix
    Mix_CloseAudio();
    Mix_Quit();
    // 退出 TTF
    TTF_Quit();
    // 退出 SDL
    SDL_Quit();
}

void Game::run() {
    constexpr auto kFrameTime = 1s / kFps;
    auto last = std::chrono::steady_clock::now();  // 上一帧update时间
    while (is_running_) {
        auto start = std::chrono::steady_clock::now();  // 当前帧时间

        if (next_scene_) {
            if (current_scene_) {
                current_scene_->clean();
            }
            current_scene_ = std::move(next_scene_);
            if (current_scene_) {
                current_scene_->init();
            }
#ifdef DEBUG_MODE
            fmt::println("Change scene {}", current_scene_->getName());
#endif
        }

        handleEvents();
        update(start - last);
        last = start;
        render();

        auto end = std::chrono::steady_clock::now();
        auto elapsed = end - last;
        if (elapsed < kFrameTime) {
            std::this_thread::sleep_for(kFrameTime - elapsed - 0.1ms);
        }

        // {
        //     auto tmp = std::chrono::steady_clock::now();
        //     fmt::println("elapsed: {},  FPS: {}", elapsed.count(), 1.0s / (tmp - start));
        // }
    }

    clean();
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                is_running_ = false;
                break;
            default:
                current_scene_->handleEvents(event);
                break;
        }
    }
}

void Game::update(std::chrono::duration<float> delta) {
    mouse_button_flags_ = SDL_GetMouseState(&mouse_position_.x, &mouse_position_.y);
    current_scene_->update(delta);
}

void Game::render() {
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderClear(renderer_);
    current_scene_->render();
    SDL_RenderPresent(renderer_);
}

void Game::changeScene(std::unique_ptr<Scene> scene) { next_scene_ = std::move(scene); }

void Game::setScore(int score) {
    score_ = score;
    high_score_ = std::max(high_score_, score_);
}

void Game::addScore(int score) { setScore(score_ + score); }

void Game::renderTexture(const Texture& texture, const glm::vec2& position, const glm::vec2& size,
                         const glm::vec2 mask) const {
    SDL_FRect src_rect = {
        texture.src_rect.x,
        texture.src_rect.y + texture.src_rect.h * (1 - mask.y),
        texture.src_rect.w * mask.x,
        texture.src_rect.h * mask.y,
    };
    SDL_FRect dst_rect = {
        position.x,
        position.y + size.y * (1 - mask.y),
        size.x * mask.x,
        size.y * mask.y,
    };
    SDL_RenderTextureRotated(renderer_, texture.texture, &src_rect, &dst_rect, texture.angle, nullptr,
                             texture.is_flip ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void Game::renderFillCircle(const std::string& file_path, const glm::vec2& position, const glm::vec2& size,
                            float alpha) const {
    auto texture = asset_store_->getImage(file_path);
    SDL_FRect dst_rect = {
        position.x,
        position.y,
        size.x,
        size.y,
    };
    SDL_SetTextureAlphaModFloat(texture, alpha);
    SDL_RenderTexture(renderer_, texture, nullptr, &dst_rect);
}

void Game::renderHBar(const glm::vec2& position, const glm::vec2& size, float percent, SDL_FColor color) const {
    SDL_SetRenderDrawColorFloat(renderer_, color.r, color.g, color.b, color.a);
    SDL_FRect boundary_rect = {
        position.x,
        position.y,
        size.x,
        size.y,
    };
    SDL_FRect fill_rect = {
        position.x,
        position.y,
        size.x * percent,
        size.y,
    };
    SDL_RenderRect(renderer_, &boundary_rect);
    SDL_RenderFillRect(renderer_, &fill_rect);
    SDL_SetRenderDrawColorFloat(renderer_, 0, 0, 0, 1);
}

TTF_Text* Game::createTTF_Text(std::string_view text, const std::string& font_path, int font_size) const {
    auto font = asset_store_->getFont(font_path, font_size);
    return TTF_CreateText(ttf_engine_, font, text.data(), text.size());
}

void Game::drawGrid(const glm::vec2& top_left, const glm::vec2& bottom_right, float grid_width, float grid_height,
                    SDL_FColor color) const {
    SDL_SetRenderDrawColorFloat(renderer_, color.r, color.g, color.b, color.a);
    for (float x = top_left.x; x <= bottom_right.x; x += grid_width) {
        SDL_RenderLine(renderer_, x, top_left.y, x, bottom_right.y);
    }
    for (float y = top_left.y; y <= bottom_right.y; y += grid_height) {
        SDL_RenderLine(renderer_, top_left.x, y, bottom_right.x, y);
    }
    SDL_SetRenderDrawColorFloat(renderer_, 0, 0, 0, 1);
}

void Game::drawBoundary(const glm::vec2& top_left, const glm::vec2& bottom_right, float boundary_width,
                        SDL_FColor color) const {
    SDL_SetRenderDrawColorFloat(renderer_, color.r, color.g, color.b, color.a);
    for (float i = 0; i < boundary_width; i++) {
        SDL_FRect rect = {
            top_left.x - i,
            top_left.y - i,
            bottom_right.x - top_left.x + 2 * i,
            bottom_right.y - top_left.y + 2 * i,
        };
        SDL_RenderRect(renderer_, &rect);
    }
    SDL_SetRenderDrawColorFloat(renderer_, 0, 0, 0, 1);
}

bool Game::isInRect(const glm::vec2& position, const glm::vec2& top_left, const glm::vec2& bottom_right) const {
    return position.x >= top_left.x && position.x <= bottom_right.x && position.y >= top_left.y &&
           position.y <= bottom_right.y;
}

bool Game::isMouseInRect(const glm::vec2& top_left, const glm::vec2& bottom_right) const {
    return isInRect(getMousePosition(), top_left, bottom_right);
}

}  // namespace sdl3
}  // namespace pyc