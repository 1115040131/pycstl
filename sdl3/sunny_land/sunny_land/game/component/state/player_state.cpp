module;

#include <string_view>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

module sunny_land.game.player;

import common.string_hash;
import sunny_land.engine.component.animation_component;
import sunny_land.engine.component.audio_component;
import sunny_land.engine.core;

namespace pyc::sunny_land {

void PlayerState::playAnimation(std::string_view animation_name) {
    if (!player_component_) {
        spdlog::error("PlayerState 没有关联的 PlayerComponent, 无法播放动画 '{}'", animation_name);
        return;
    }

    auto animation_component = player_component_->getAnimationComponent();
    if (!animation_component) {
        spdlog::error("PlayerComponent '{}' 没有 AnimationComponent, 无法播放动画 '{}'",
                      player_component_->getOwner()->getName(), animation_name);
        return;
    }

    animation_component->playAnimation(animation_name);
}

void PlayerState::playSound(std::string_view audio_name) {
    if (!player_component_) {
        spdlog::error("PlayerState 没有关联的 PlayerComponent, 无法播放音效 '{}'", audio_name);
        return;
    }

    auto audio_component = player_component_->getAudioComponent();
    if (!audio_component) {
        spdlog::error("PlayerComponent '{}' 没有 AudioComponent, 无法播放音效 '{}'",
                      player_component_->getOwner()->getName(), audio_name);
        return;
    }

    audio_component->playSound(audio_name);
}

}  // namespace pyc::sunny_land
