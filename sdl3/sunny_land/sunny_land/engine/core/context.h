#pragma once

#include "common/noncopyable.h"

namespace pyc::sunny_land {

class ResourceManager;
class Renderer;
class Camera;
class InputManager;
class PhysicsEngine;
class AudioPlayer;

class Context final : Noncopyable {
public:
    /**
     * @brief 构造函数。
     * @param resource_manager 对 ResourceManager 实例的引用。
     * @param renderer 对 Renderer 实例的引用。
     * @param camera 对 Camera 实例的引用。
     * @param input_manager 对 InputManager 实例的引用。
     * @param physics_engine 对 PhysicsEngine 实例的引用。
     */
    Context(ResourceManager& resource_manager, Renderer& renderer, Camera& camera, InputManager& input_manager,
            PhysicsEngine& physics_engine, AudioPlayer& audio_player);

    // --- Getters ---
    ResourceManager& getResourceManager() const { return resource_manager_; }  ///< @brief 获取资源管理器
    Renderer& getRenderer() const { return renderer_; }                        ///< @brief 获取渲染器
    Camera& getCamera() const { return camera_; }                              ///< @brief 获取相机
    InputManager& getInputManager() const { return input_manager_; }           ///< @brief 获取输入管理器
    PhysicsEngine& getPhysicsEngine() const { return physics_engine_; }        ///< @brief 获取物理引擎
    AudioPlayer& getAudioPlayer() const { return audio_player_; }              ///< @brief 获取音频播放器

private:
    // 使用引用，确保每个模块都有效，使用时不需要检查指针是否为空。
    ResourceManager& resource_manager_;  ///< @brief 资源管理器
    Renderer& renderer_;                 ///< @brief 渲染器
    Camera& camera_;                     ///< @brief 相机
    InputManager& input_manager_;        ///< @brief 输入管理器
    PhysicsEngine& physics_engine_;      ///< @brief 物理引擎
    AudioPlayer& audio_player_;          ///< @brief 音频播放器
};

}  // namespace pyc::sunny_land