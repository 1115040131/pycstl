#pragma once

#include "sunny_land/engine/ui/state/ui_state.h"

namespace pyc::sunny_land {

/**
 * @brief 按下状态
 *
 * 当鼠标按下UI元素时，会切换到该状态。
 */
class UIHoverState final : public UIState {
    friend class UIInteractive;

public:
    UIHoverState(UIInteractive* owner) : UIState(owner) {}
    ~UIHoverState() override = default;

private:
    void enter() override;
    std::unique_ptr<UIState> handleInput(Context& context) override;
};

}  // namespace pyc::sunny_land