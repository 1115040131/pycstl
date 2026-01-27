#pragma once

#include <entt/signal/fwd.hpp>

#include "common/noncopyable.h"

namespace pyc::monster_war {

class ResourceManager;
class Renderer;
class Camera;
class TextRenderer;
class InputManager;
class AudioPlayer;
class GameState;
class Time;

class Context final : Noncopyable {
public:
    // clang-format off
    /**
     * @brief 构造函数。
     * @param dispatcher 对事件分发器的引用。
     * @param input_manager 对 InputManager 实例的引用。
     * @param renderer 对 Renderer 实例的引用。
     * @param camera 对 Camera 实例的引用。
     * @param text_renderer 对 TextRenderer 实例的引用。
     * @param resource_manager 对 ResourceManager 实例的引用。
     * @param audio_player 对 AudioPlayer 实例的引用。
     * @param game_state 对 GameState 实例的引用。
     * @param time 对 Time 实例的引用。
     */
    Context(entt::dispatcher& dispatcher,
            ResourceManager& resource_manager,
            Renderer& renderer,
            Camera& camera,
            TextRenderer& text_renderer,
            InputManager& input_manager,
            AudioPlayer& audio_player,
            GameState& game_state,
            Time& time);
    // clang-format on

    // --- Getters ---
    entt::dispatcher& getDispatcher() const { return dispatcher_; }            ///< @brief 获取事件分发器
    ResourceManager& getResourceManager() const { return resource_manager_; }  ///< @brief 获取资源管理器
    Renderer& getRenderer() const { return renderer_; }                        ///< @brief 获取渲染器
    Camera& getCamera() const { return camera_; }                              ///< @brief 获取相机
    TextRenderer& getTextRenderer() const { return text_renderer_; }           ///< @brief 获取文本渲染器
    InputManager& getInputManager() const { return input_manager_; }           ///< @brief 获取输入管理器
    AudioPlayer& getAudioPlayer() const { return audio_player_; }              ///< @brief 获取音频播放器
    GameState& getGameState() const { return game_state_; }                    ///< @brief 获取游戏状态
    Time& getTime() const { return time_; }                                    ///< @brief 获取时间

private:
    // 使用引用，确保每个模块都有效，使用时不需要检查指针是否为空。
    entt::dispatcher& dispatcher_;       ///< @brief 事件分发器
    ResourceManager& resource_manager_;  ///< @brief 资源管理器
    Renderer& renderer_;                 ///< @brief 渲染器
    Camera& camera_;                     ///< @brief 相机
    TextRenderer& text_renderer_;        ///< @brief 文本渲染器
    InputManager& input_manager_;        ///< @brief 输入管理器
    AudioPlayer& audio_player_;          ///< @brief 音频播放器
    GameState& game_state_;              ///< @brief 游戏状态
    Time& time_;                         ///< @brief 时间
};

}  // namespace pyc::monster_war