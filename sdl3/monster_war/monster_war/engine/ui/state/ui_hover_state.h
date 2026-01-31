#pragma once

#include "monster_war/engine/ui/state/ui_state.h"

namespace pyc::monster_war {

/**
 * @brief 按下状态
 *
 * 当鼠标按下UI元素时，会切换到该状态。
 */
class UIHoverState final : public UIState {
    friend class UIInteractive;

public:
    UIHoverState(UIInteractive* owner);
    ~UIHoverState() override;

private:
    void enter() override;
    void update(std::chrono::duration<float> delta_time, Context& context) override;

    bool onMousePressed();  ///< @brief 鼠标按下回调函数 (不再使用轮询“isActionPressed”)
};

}  // namespace pyc::monster_war