#pragma once

#include <chrono>

#include "common/noncopyable.h"

namespace pyc::sunny_land {

class AIComponent;

/**
 * @brief AI 行为策略的抽象基类。
 */
class AIBehavior : Noncopyable {
    friend class AIComponent;

public:
    virtual ~AIBehavior() = default;

protected:
    // --- 没有保存owner指针，因此需要传入 AIComponent 引用 ---
    virtual void enter(AIComponent&) {}  ///< @brief enter函数可选是否实现，默认为空
    virtual void update(std::chrono::duration<float>,
                        AIComponent&) = 0;  ///< @brief 更新 AI 行为逻辑(具体策略)，必须实现
};

}  // namespace pyc::sunny_land
