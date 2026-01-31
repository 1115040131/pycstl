#pragma once

#include <chrono>
#include <string>

#include <entt/entity/entity.hpp>

namespace pyc::monster_war {

/**
 * @brief 技能组件
 * @note 用于存储技能信息，包括技能ID、用于显示特效的实体ID、名称、描述、冷却时间、计时器等。
 */
struct SkillComponent {
    entt::id_type skill_id_{entt::null};                 ///< 技能ID
    entt::entity display_entity_{entt::null};            ///< 用于显示特效的实体ID
    std::string name_;                                   ///< @brief 技能名称
    std::string description_;                            ///< @brief 技能描述
    std::chrono::duration<float> cooldown_{0.0f};        ///< @brief 技能冷却时间
    std::chrono::duration<float> duration_{0.0f};        ///< @brief 技能持续时间
    std::chrono::duration<float> cooldown_timer_{0.0f};  ///< @brief 技能冷却计时器
    std::chrono::duration<float> duration_timer_{0.0f};  ///< @brief 技能持续计时器
};

}  // namespace pyc::monster_war