#include "sunny_land/engine/input/input_manager.h"

#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

#include "sunny_land/engine/core/config.h"

namespace pyc::sunny_land {

InputManager::InputManager(SDL_Renderer* sdl_renderer, Config* config) : sdl_renderer_(sdl_renderer) {
    if (!sdl_renderer_) {
        spdlog::error("输入管理器: SDL_Renderer 为空指针");
        throw std::runtime_error("输入管理器: SDL_Renderer 为空指针");
    }
    initializeMappings(config);

    // 获取鼠标初始位置
    float x{};
    float y{};
    SDL_GetMouseState(&x, &y);
}

void InputManager::update() {
    // 1. 根据上一帧的值更新默认的动作状态
    for (auto& [action_name, state] : action_states_) {
        if (state == ActionState::PRESSED_THIS_FRAME) {
            state = ActionState::HELD_DOWN;
        } else if (state == ActionState::RELEASED_THIS_FRAME) {
            state = ActionState::INACTIVE;
        }
    }

    // 2. 处理所有待处理的 SDL 事件 (这将设定 action_states_ 的值)
    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
        processEvent(event);
    }
}

bool InputManager::isActionDown(std::string_view action_name) const {
    auto it = action_states_.find(action_name);
    if (it != action_states_.end()) {
        return it->second == ActionState::PRESSED_THIS_FRAME || it->second == ActionState::HELD_DOWN;
    }
    return false;
}

bool InputManager::isActionPressed(std::string_view action_name) const {
    auto it = action_states_.find(action_name);
    if (it != action_states_.end()) {
        return it->second == ActionState::PRESSED_THIS_FRAME;
    }
    return false;
}

bool InputManager::isActionReleased(std::string_view action_name) const {
    auto it = action_states_.find(action_name);
    if (it != action_states_.end()) {
        return it->second == ActionState::RELEASED_THIS_FRAME;
    }
    return false;
}

bool InputManager::shouldQuit() const { return should_quit_; }

void InputManager::setShouldQuit(bool should_quit) { should_quit_ = should_quit; }

glm::vec2 InputManager::getMousePosition() const { return mouse_position_; }

glm::vec2 InputManager::getLogicalMousePosition() const {
    glm::vec2 logical_pos{};
    // 通过窗口坐标获取渲染坐标（逻辑坐标）
    SDL_RenderCoordinatesFromWindow(sdl_renderer_, mouse_position_.x, mouse_position_.y, &logical_pos.x,
                                    &logical_pos.y);
    return logical_pos;
}

void InputManager::processEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
            auto it = input_to_actions_map_.find(event.key.scancode);
            if (it != input_to_actions_map_.end()) {
                for (const auto& action_name : it->second) {
                    updateActionState(action_name, event.key.down, event.key.repeat);
                }
            }
        } break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP: {
            auto it = input_to_actions_map_.find(event.button.button);
            if (it != input_to_actions_map_.end()) {
                for (const auto& action_name : it->second) {
                    updateActionState(action_name, event.button.down, false);
                }
            }
            // 在点击时更新鼠标位置
            mouse_position_ = {event.button.x, event.button.y};
        } break;
        case SDL_EVENT_MOUSE_MOTION:  // 处理鼠标运动
            mouse_position_ = {event.motion.x, event.motion.y};
            break;
        case SDL_EVENT_QUIT:
            should_quit_ = true;
            break;
        default:
            break;
    }
}

void InputManager::initializeMappings(Config* config) {
    spdlog::trace("初始化输入映射...");
    if (!config) {
        spdlog::error("输入管理器: Config 为空指针");
        throw std::runtime_error("输入管理器: Config 为空指针");
    }

    actions_to_keyname_map_ = config->CONFIG(input_mappings);
    input_to_actions_map_.clear();

    // 如果配置中没有定义鼠标按钮动作(通常不需要配置),则添加默认映射, 用于 UI
    if (actions_to_keyname_map_.find("MouseLeftClick") == actions_to_keyname_map_.end()) {
        spdlog::debug("配置中没有定义 'MouseLeftClick' 动作,添加默认映射到 'MouseLeft'.");
        actions_to_keyname_map_["MouseLeftClick"] = {"MouseLeft"};  // 如果缺失则添加默认映射
    }
    if (actions_to_keyname_map_.find("MouseRightClick") == actions_to_keyname_map_.end()) {
        spdlog::debug("配置中没有定义 'MouseRightClick' 动作,添加默认映射到 'MouseRight'.");
        actions_to_keyname_map_["MouseRightClick"] = {"MouseRight"};  // 如果缺失则添加默认映射
    }

    // 遍历 动作 -> 按键名称 的映射
    for (const auto& [action_name, key_names] : actions_to_keyname_map_) {
        // 每个动作对应一个动作状态，初始化为 INACTIVE
        action_states_[action_name] = ActionState::INACTIVE;
        spdlog::trace("映射动作: {}", action_name);
        // 设置 "按键 -> 动作" 的映射
        for (const auto& key_name : key_names) {
            if (auto scancode = scancodeFromString(key_name)) {
                // 如果 scancode 有效, 则将 action 添加到 input_to_actions_map_ 中
                input_to_actions_map_[scancode.value()].push_back(action_name);
                spdlog::trace("  映射按键: {} (Scancode: {}) 到动作: {}", key_name,
                              static_cast<int>(scancode.value()), action_name);
            } else if (auto mouse_button = mouseButtonFromString(key_name)) {
                // 如果鼠标按钮有效, 则将 action 添加到 input_to_actions_map_ 中
                input_to_actions_map_[mouse_button.value()].push_back(action_name);
                spdlog::trace("  映射鼠标按钮: {} (Button ID: {}) 到动作: {}", key_name, mouse_button.value(),
                              action_name);
                // else if: 未来可添加其它输入类型 ...
            } else {
                spdlog::warn("输入映射警告: 未知键或按钮名称 '{}' 用于动作 '{}'.", key_name, action_name);
            }
        }
    }
    spdlog::trace("输入映射初始化完成.");
}

void InputManager::updateActionState(std::string_view action_name, bool is_input_active, bool is_repeat_event) {
    auto it = action_states_.find(action_name);
    if (it == action_states_.end()) {
        spdlog::warn("尝试更新未注册的动作状态: {}", action_name);
        return;
    }

    if (is_input_active) {
        if (is_repeat_event) {
            it->second = ActionState::HELD_DOWN;
        } else {
            it->second = ActionState::PRESSED_THIS_FRAME;
        }
    } else {
        it->second = ActionState::RELEASED_THIS_FRAME;
    }
}

std::optional<SDL_Scancode> InputManager::scancodeFromString(std::string_view key_name) {
    auto scancode = SDL_GetScancodeFromName(key_name.data());
    if (scancode != SDL_SCANCODE_UNKNOWN) {
        return scancode;
    }
    return std::nullopt;
}

std::optional<uint32_t> InputManager::mouseButtonFromString(std::string_view button_name) {
    if (button_name == "MouseLeft") {
        return SDL_BUTTON_LEFT;
    } else if (button_name == "MouseMiddle") {
        return SDL_BUTTON_MIDDLE;
    } else if (button_name == "MouseRight") {
        return SDL_BUTTON_RIGHT;
    } else if (button_name == "MouseX1") {
        return SDL_BUTTON_X1;
    } else if (button_name == "MouseX2") {
        return SDL_BUTTON_X2;
    }
    return std::nullopt;
}

}  // namespace pyc::sunny_land