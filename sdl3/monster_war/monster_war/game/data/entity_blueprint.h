#pragma once

#include <chrono>
#include <unordered_map>

#include <entt/entity/entity.hpp>
#include <glm/vec2.hpp>

#include "monster_war/engine/utils/math.h"

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
    std::vector<int> frames_;  ///< @brief 动画帧索引数组
};

/// @brief 声音蓝图, 用于创建声音组件
struct SoundBlueprint {
    std::unordered_map<entt::id_type, entt::id_type> sounds_;
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

/// @brief 敌人类型蓝图, 包含所有必要的子蓝图，用于创建敌人实体中的所有组件
struct EnemyClassBlueprint {
    entt::id_type class_id_{entt::null};
    std::string class_name_;
    StatsBlueprint stats_{};
    EnemyBlueprint enemy_{};
    SoundBlueprint sounds_{};
    SpriteBlueprint sprite_{};
    DisplayInfoBlueprint display_info_{};
    std::unordered_map<entt::id_type, AnimationBlueprint> animations_;
};

}  // namespace pyc::monster_war