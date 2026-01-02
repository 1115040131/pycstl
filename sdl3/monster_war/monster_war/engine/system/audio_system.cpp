#include "monster_war/engine/system/audio_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/audio/audio_player.h"
#include "monster_war/engine/component/audio_component.h"
#include "monster_war/engine/core/context.h"

namespace pyc::monster_war {

AudioSystem::AudioSystem(entt::registry& registry, Context& context) : registry_(registry), context_(context) {
    auto& dispatcher = context_.getDispatcher();
    dispatcher.sink<PlaySoundEvent>().connect<&AudioSystem::onPlaySoundEvent>(this);
}

AudioSystem::~AudioSystem() {
    auto& dispatcher = context_.getDispatcher();
    dispatcher.sink<PlaySoundEvent>().disconnect<&AudioSystem::onPlaySoundEvent>(this);
}

void AudioSystem::onPlaySoundEvent(const PlaySoundEvent& event) {
    // 如果没有传入目标实体，则直接播放全局音效
    if (event.entity_ == entt::null) {
        spdlog::info("播放全局音效: {}", event.sound_id_);
        context_.getAudioPlayer().playSound(event.sound_id_);
    } else if (auto audio = registry_.try_get<AudioComponent>(event.entity_)) {  // 如果有传入实体有音效组件
        auto it = audio->sounds_.find(event.sound_id_);
        if (it != audio->sounds_.end()) {  // 先尝试在目标实体的音效集合中查找
            spdlog::info("实体 ID: {} 中找到了音效: {}", entt::to_integral(event.entity_), it->second);
            context_.getAudioPlayer().playSound(it->second);
        } else {  // 如果没找到，则播放全局音效
            spdlog::info("实体 ID: {} 中没有找到音效: {}", entt::to_integral(event.entity_), event.sound_id_);
            context_.getAudioPlayer().playSound(event.sound_id_);
        }
    } else {  // 如果有传入目标实体，但实体没有音效组件，也尝试播放全局音效
        spdlog::info("实体 ID: {} 中没有音效组件，尝试播放全局音效: {}", entt::to_integral(event.entity_),
                     event.sound_id_);
        context_.getAudioPlayer().playSound(event.sound_id_);
    }
}

}  // namespace pyc::monster_war