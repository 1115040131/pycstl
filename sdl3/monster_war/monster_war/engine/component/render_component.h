#pragma once

#include <compare>

namespace pyc::monster_war {

/**
 * @brief 渲染组件, 包含图层ID和深度。
 */
struct RenderComponent {
    int layer{};    ///< @brief 图层ID，数字越小越先绘制
    float depth{};  ///< @brief 在同一图层内的深度，数字越小越先绘制
                    /*  (可用于实现y-sort排序，也可设定其它渲染顺序逻辑) */
    // TODO: 未来可添加其他信息，比如透明度等 ...

    // 重载比较运算符，用于排序
    constexpr auto operator<=>(const RenderComponent& other) const = default;
};

}  // namespace pyc::monster_war