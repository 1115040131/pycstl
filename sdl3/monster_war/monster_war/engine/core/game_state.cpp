#include "monster_war/engine/core/game_state.h"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

namespace pyc::monster_war {

GameState::GameState(SDL_Window* window, SDL_Renderer* renderer, State initial_state)
    : window_(window), renderer_(renderer), current_state_(initial_state) {
    if (window_ == nullptr || renderer_ == nullptr) {
        spdlog::error("窗口或渲染器为空");
        throw std::runtime_error("窗口或渲染器不能为空");
    }
    // 记录初始逻辑分辨率(由 GameApp::initSDL 在构造本类之前设置好)
    SDL_GetRenderLogicalPresentation(renderer_, &logical_size_.x, &logical_size_.y, NULL);
    spdlog::trace("游戏状态初始化完成");
}

void GameState::setState(State new_state) {
    if (current_state_ != new_state) {
        spdlog::debug("游戏状态改变");
        current_state_ = std::move(new_state);
    } else {
        spdlog::debug("尝试设置相同的游戏状态，跳过");
    }
}

glm::vec2 GameState::getWindowSize() const {
    int width{};
    int height{};
    // SDL3获取窗口大小的方法
    SDL_GetWindowSize(window_, &width, &height);
    return glm::vec2(width, height);
}

void GameState::setWindowSize(const glm::vec2& window_size) {
    SDL_SetWindowSize(window_, static_cast<int>(window_size.x), static_cast<int>(window_size.y));
}

glm::vec2 GameState::getLogicalSize() const {
    // 返回缓存值：逻辑分辨率被临时关闭期间(见 disableLogicalPresentation)，
    // SDL_GetRenderLogicalPresentation 会读回 0x0
    return glm::vec2(logical_size_);
}

void GameState::setLogicalSize(const glm::vec2& logical_size) {
    logical_size_ = glm::ivec2(logical_size);
    SDL_SetRenderLogicalPresentation(renderer_, logical_size_.x, logical_size_.y,
                                     SDL_LOGICAL_PRESENTATION_LETTERBOX);
    spdlog::trace("逻辑分辨率设置为: {}x{}", logical_size_.x, logical_size_.y);
}

bool GameState::disableLogicalPresentation() {
    return SDL_SetRenderLogicalPresentation(renderer_, logical_size_.x, logical_size_.y,
                                            SDL_LOGICAL_PRESENTATION_DISABLED);
}

bool GameState::enableLogicalPresentation() {
    return SDL_SetRenderLogicalPresentation(renderer_, logical_size_.x, logical_size_.y,
                                            SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

}  // namespace pyc::monster_war