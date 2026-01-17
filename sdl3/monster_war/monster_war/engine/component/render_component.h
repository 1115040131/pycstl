#pragma once

#include "monster_war/engine/utils/math.h"

namespace pyc::monster_war {

/**
 * @brief 渲染组件, 包含图层ID和深度，
 * 颜色调整参数（调整后 = 原始颜色 * 调整颜色）
 */
struct RenderComponent {
    static constexpr int kMainLayer{10};  ///< @brief 主图层ID，默认为10

    int layer{kMainLayer};           ///< @brief 图层ID，数字越小越先绘制
    float depth{};                   ///< @brief 在同一图层内的深度，数字越小越先绘制
                                     /*  (可用于实现y-sort排序，也可设定其它渲染顺序逻辑) */
    FColor color_{FColor::white()};  ///< @brief 颜色调整参数

    // 重载比较运算符，用于排序
    constexpr auto operator<=>(const RenderComponent& other) const {
        return std::tie(layer, depth) <=> std::tie(other.layer, other.depth);
    }

    bool operator==(const RenderComponent&) const = default;
};

}  // namespace pyc::monster_war