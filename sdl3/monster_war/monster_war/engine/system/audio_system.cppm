module;

#include <entt/entity/fwd.hpp>

export module monster_war.engine.system.audio_system;

export import monster_war.engine.core.context;
export import monster_war.engine.utils.events;

export namespace pyc::monster_war {

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
