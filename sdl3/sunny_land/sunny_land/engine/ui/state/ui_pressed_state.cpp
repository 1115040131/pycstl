#include "sunny_land/engine/ui/state/ui_pressed_state.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/engine/ui/state/ui_state_factory.h"
#include "sunny_land/engine/ui/ui_interactive.h"

namespace pyc::sunny_land {

void UIPressedState::enter() {
    owner_->setSprite("pressed");
    owner_->playSound("pressed");
    spdlog::debug("切换到按下状态");
}

std::unique_ptr<UIState> UIPressedState::handleInput(Context& context) {
    auto& input_manager = context.getInputManager();
    auto mouse_pos = input_manager.getLogicalMousePosition();
    if (input_manager.isActionReleased("MouseLeftClick")) {
        if (!owner_->isPointInside(mouse_pos)) {  // 松开鼠标时，如果不在UI元素内，则切换到正常状态
            return UIStateFactory::create<UINormalState>(owner_);
        } else {  // 松开鼠标时，如果还在UI元素内，则触发点击事件
            owner_->clicked();
            return UIStateFactory::create<UIHoverState>(owner_);
        }
    }

    return nullptr;
}

}  // namespace pyc::sunny_land