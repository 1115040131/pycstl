#pragma once

#include <chrono>
#include <unordered_map>

#include <entt/entity/entity.hpp>
#include <glm/vec2.hpp>

#include "monster_war/engine/utils/math.h"
#include "monster_war/game/def/constants.h"

namespace pyc::monster_war {

/// @brief 属性蓝图
struct StatsBlueprint {
    float hp_{};
    float atk_{};
    float def_{};
    float range_{};
    std::chrono::duration<float> atk_interval_{};
};

/// @brief 精灵蓝图
struct SpriteBlueprint {
    entt::id_type id_{entt::null};
    std::string path_;
    Rect src_rect_{};
    glm::vec2 size_{0.0f};
    glm::vec2 offset_{0.0f};
    bool face_right_{true};  ///< @brief 角色图片默认朝右，如果朝左就设置为false
};

/// @brief 单一动画的蓝图，多个蓝图构成的关联容器即可用于创建动画组件
struct AnimationBlueprint {
    std::chrono::duration<float> per_frame_{0.0f};
    int row_{0};
    std::vector<size_t> frames_;                        ///< @brief 动画帧索引数组
    std::unordered_map<size_t, entt::id_type> events_;  ///< @brief 动画事件，键为帧索引，值为事件ID
};

/// @brief 声音蓝图, 用于创建声音组件
struct SoundBlueprint {
    std::unordered_map<entt::id_type, entt::id_type> sounds_;
};

/// @brief 玩家蓝图, 用于创建玩家组件、放置类型、阻挡数量等
struct PlayerBlueprint {
    PlayerType type_{PlayerType::UNKNOWN};
    entt::id_type skill_id_{entt::null};
    bool healer_{false};
    int block_{0};
    int cost_{0};
};

/// @brief 敌人蓝图, 用于创建敌人组件（EnemyComponent）
struct EnemyBlueprint {
    bool ranged_{false};
    float speed_{};
};

/// @brief 显示信息蓝图, 可用于查找对应职业的名称和描述
struct DisplayInfoBlueprint {
    std::string name_;
    std::string description_;
};

/// @brief 玩家职业蓝图, 包含所有必要的子蓝图，用于创建玩家实体中的所有组件
struct PlayerClassBlueprint {
    entt::id_type class_id_{entt::null};
    entt::id_type projectile_id_{entt::null};
    std::string class_name_;
    StatsBlueprint stats_{};
    PlayerBlueprint player_{};
    SoundBlueprint sounds_{};
    SpriteBlueprint sprite_{};
    DisplayInfoBlueprint display_info_{};
    std::unordered_map<entt::id_type, AnimationBlueprint> animations_;
};

/// @brief 敌人类型蓝图, 包含所有必要的子蓝图，用于创建敌人实体中的所有组件
struct EnemyClassBlueprint {
    entt::id_type class_id_{entt::null};
    entt::id_type projectile_id_{entt::null};
    std::string class_name_;
    StatsBlueprint stats_{};
    EnemyBlueprint enemy_{};
    SoundBlueprint sounds_{};
    SpriteBlueprint sprite_{};
    DisplayInfoBlueprint display_info_{};
    std::unordered_map<entt::id_type, AnimationBlueprint> animations_;
};

/// @brief 投射物蓝图, 用于创建投射物组件
struct ProjectileBlueprint {
    entt::id_type id_{entt::null};
    std::string name_;
    float arc_height_{};
    std::chrono::duration<float> total_flight_time_{};
    SpriteBlueprint sprite_{};
    SoundBlueprint sounds_{};
};

}  // namespace pyc::monster_war