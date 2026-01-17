#pragma once

#include <entt/entity/entity.hpp>

#include "monster_war/game/def/constants.h"

namespace pyc::monster_war {

/**
 * @brief 单位准备组件，存储单位准备信息
 * @note 包括名称、类型、范围、费用
 */
struct UnitPrepComponent {
    entt::id_type name_id_{entt::null};
    PlayerType type_{PlayerType::UNKNOWN};
    float range_{0};
    int cost_{0};
};

}  // namespace pyc::monster_war