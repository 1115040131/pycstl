#pragma once

#include <glm/vec2.hpp>

namespace pyc::monster_war {

/**
 * @brief 视差组件，包含滚动速度因子、是否重复和是否可见。（需和Sprite配合使用）
 */
struct ParallaxComponent {
    glm::vec2 scroll_factor_{};  ///< @brief 滚动速度因子 (0=静止, 1=随相机移动, <1=比相机慢)
    glm::bvec2 repeat_{true};    ///< @brief 是否重复
    bool is_visible_{true};      ///< @brief 是否可见
};

}  // namespace pyc::monster_war