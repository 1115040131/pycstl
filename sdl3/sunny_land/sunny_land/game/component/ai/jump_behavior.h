#pragma once

#include <glm/glm.hpp>

#include "sunny_land/game/component/ai/ai_behavior.h"

namespace pyc::sunny_land {

using namespace std::chrono_literals;

/**
 * @brief AI 行为：在指定范围内周期性地跳跃。
 *
 * 在地面时等待，然后向当前方向跳跃。
 * 撞墙或到达边界时改变下次跳跃方向。
 */
class JumpBehavior final : public AIBehavior {
    friend class AIComponent;

public:
    /**
     * @brief 构造函数。
     * @param min_x 巡逻范围的最小 x 坐标。
     * @param max_x 巡逻范围的最大 x 坐标。
     * @param jump_vel 跳跃速度向量 (水平, 垂直)。
     * @param jump_interval 两次跳跃之间的间隔时间。
     */
    JumpBehavior(float min_x, float max_x, glm::vec2 jump_vel = {100.0f, -300.0f},
                 std::chrono::duration<float> jump_interval = 2s);

private:
    void update(std::chrono::duration<float> delta_time, AIComponent& ai_component) override;

private:
    float patrol_min_x_ = 0.0f;                        ///< @brief 巡逻范围的左边界
    float patrol_max_x_ = 0.0f;                        ///< @brief 巡逻范围的右边界
    glm::vec2 jump_vel_ = glm::vec2(100.0f, -300.0f);  ///< @brief 跳跃速度
    std::chrono::duration<float> jump_interval_{2s};   ///< @brief 跳跃间隔时间 (秒)
    std::chrono::duration<float> jump_timer_{};        ///< @brief 距离下次跳跃的计时器
    bool jumping_right_ = false;                       ///< @brief 当前是否向右跳跃
};

}  // namespace pyc::sunny_land
