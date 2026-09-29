module;

#include <unordered_map>

#include <entt/entity/fwd.hpp>

export module monster_war.engine.component.audio_component;

export namespace pyc::monster_war {

/**
 * @brief 音频组件，包含音效集合。
 */
struct AudioComponent {
    std::unordered_map<entt::id_type, entt::id_type> sounds_;  ///< @brief 音效集合，名称(哈希) -> 音效ID
};

}  // namespace pyc::monster_war
