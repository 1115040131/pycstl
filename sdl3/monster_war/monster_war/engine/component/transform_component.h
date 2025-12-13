#pragma once

#include <glm/vec2.hpp>

namespace pyc::monster_war {

/**
 * @brief 变换组件，包含位置、缩放和旋转。
 */
struct TransformComponent {
    glm::vec2 position_ = {0.0f, 0.0f};  ///< @brief 位置
    glm::vec2 scale_ = {1.0f, 1.0f};     ///< @brief 缩放
    float rotation_ = 0.0f;              ///< @brief 角度制，单位：度
};

}  // namespace pyc::monster_war