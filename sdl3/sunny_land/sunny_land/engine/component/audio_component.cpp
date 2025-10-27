#include "sunny_land/engine/component/audio_component.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/audio/audio_player.h"
#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/render/camera.h"

namespace pyc::sunny_land {

AudioComponent::AudioComponent(AudioPlayer* audio_player, Camera* camera)
    : audio_player_(audio_player), camera_(camera) {
    if (!audio_player_ || !camera_) {
        spdlog::error("AudioComponent 初始化失败: 音频播放器或相机为空");
    }
}

void AudioComponent::init() {
    if (!owner_) {
        spdlog::error("AudioComponent 初始化失败: 所属对象为空");
        return;
    }
    transform_ = owner_->getComponent<TransformComponent>();
    if (!transform_) {
        spdlog::warn("AudioComponent 所在的 GameObject 上没有 TransformComponent! 无法进行空间定位");
    }
}

void AudioComponent::playSound(std::string_view sound_id, int channel, bool use_spatial) {
    auto it = sound_id_to_path_.find(sound_id);
    auto sound_path = (it != sound_id_to_path_.end()) ? it->second : std::string(sound_id);

    if (use_spatial && transform_) {  // 使用空间定位
        // TODO: (SDL_Mixer 不支持空间定位，未来更换音频库时可以方便地实现)
        // 这里给一个简单的功能：150像素范围内播放，否则不播放
        auto camera_position = camera_->getPosition() + camera_->getViewportSize() * 0.5f;
        auto object_position = transform_->getPosition();
        if (glm::length(camera_position - object_position) > 150.0f) {
            spdlog::debug("AudioComponent::playSound: 音效 '{}' 超出范围，不播放。", sound_id);
            return;  // 超出范围，不播放
        }
    }
    audio_player_->playSound(sound_path, channel);
}

void AudioComponent::addSound(std::string_view sound_id, std::string_view sound_path) {
    auto it = sound_id_to_path_.find(sound_id);
    if (it != sound_id_to_path_.end()) {
        spdlog::warn("AudioComponent::addSound: 音效 ID '{}' 已存在，覆盖旧路径。", sound_id);
        it->second = std::string(sound_path);
    } else {
        sound_id_to_path_.emplace(std::string(sound_id), std::string(sound_path));
    }
    spdlog::debug("AudioComponent::addSound: 添加音效 ID '{}' 路径 '{}'", sound_id, sound_path);
}

};  // namespace pyc::sunny_land