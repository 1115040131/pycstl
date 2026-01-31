#pragma once

#include <entt/entity/fwd.hpp>

#include "monster_war/engine/utils/events.h"

namespace pyc::monster_war {

class Context;

/**
 * @brief 音频系统，负责处理播放音频事件。
 */
class AudioSystem {
public:
    AudioSystem(entt::registry& registry, Context& context);
    ~AudioSystem();

private:
    void onPlaySoundEvent(const PlaySoundEvent& event);

private:
    entt::registry& registry_;
    Context& context_;
};

}  // namespace pyc::monster_war