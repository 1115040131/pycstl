#pragma once

#include <glm/vec2.hpp>

namespace pyc::monster_war {

/**
 * @brief 速度组件。
 */
struct VelocityComponent {
    glm::vec2 velocity_{};  ///< @brief 速度
};

}  // namespace pyc::monster_war