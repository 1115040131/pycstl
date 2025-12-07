#pragma once

#include "monster_war/engine/ui/state/ui_state.h"

namespace pyc::monster_war {

/**
 * @brief 按下状态
 *
 * 当鼠标按下UI元素时，会切换到该状态。
 */
class UIPressedState final : public UIState {
    friend class UIInteractive;

public:
    UIPressedState(UIInteractive* owner) : UIState(owner) {}
    ~UIPressedState() override = default;

private:
    void enter() override;
    std::unique_ptr<UIState> handleInput(Context& context) override;
};

}  // namespace pyc::monster_war