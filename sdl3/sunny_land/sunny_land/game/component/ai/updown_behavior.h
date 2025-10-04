#pragma once

#include "sunny_land/game/component/ai/ai_behavior.h"

namespace pyc::sunny_land {

/**
 * @brief AI 行为：在指定范围内上下垂直移动。
 *
 * 到达边界或碰到障碍物时会反向。
 */
class UpDownBehavior final : public AIBehavior {
    friend class AIComponent;

public:
    /**
     * @brief 构造函数。
     * @param min_y 巡逻范围的最小 y 坐标。
     * @param max_y 巡逻范围的最大 y 坐标。
     * @param speed 移动速度。
     */
    UpDownBehavior(float min_y, float max_y, float speed = 50.0f);

private:
    void enter(AIComponent& ai_component) override;
    void update(std::chrono::duration<float> delta_time, AIComponent& ai_component) override;

private:
    float patrol_min_y_ = 0.0f;  ///< @brief 巡逻范围的上边界 (Y 坐标较小值)
    float patrol_max_y_ = 0.0f;  ///< @brief 巡逻范围的下边界 (Y 坐标较大值)
    float move_speed_ = 50.0f;   ///< @brief 移动速度 (像素/秒)
    bool moving_down_ = false;   ///< @brief 当前是否向下移动
};

}  // namespace pyc::sunny_land
