#pragma once

#include <unordered_map>

#include "common/string_hash.h"
#include "sunny_land/engine/component/component.h"

namespace pyc::sunny_land {

class AudioPlayer;
class Camera;
class TransformComponent;

/**
 * @brief 音频组件，用于处理音频播放和管理。
 */
class AudioComponent final : public Component {
    friend class GameObject;

public:
    AudioComponent(AudioPlayer* audio_player, Camera* camera);

    /**
     * @brief 播放音效。
     * @param sound_path 音效文件的id (或路径)。
     * @param channel 要播放的特定通道，或 -1 表示第一个可用通道。
     * @param use_spatial 是否使用空间定位。
     */
    void playSound(std::string_view sound_id, int channel = -1, bool use_spatial = false);

    /**
     * @brief 添加音效到映射表。
     * @param sound_id 音效的标识符（针对本组件唯一即可）。
     * @param sound_path 音效文件的路径。
     */
    void addSound(std::string_view sound_id, std::string_view sound_path);

private:
    // 核心循环方法
    void init() override;

private:
    AudioPlayer* audio_player_;                ///< @brief 音频播放器的非拥有指针
    Camera* camera_;                           ///< @brief 相机的非拥有指针，用于音频空间定位
    TransformComponent* transform_ = nullptr;  ///< @brief 缓存变换组件

    ///< @brief 音效id 到路径的映射表
    std::unordered_map<std::string, std::string, StringHash, StringEqual> sound_id_to_path_;
};

}  // namespace pyc::sunny_land