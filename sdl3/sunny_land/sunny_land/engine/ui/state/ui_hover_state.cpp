#include "sunny_land/engine/ui/state/ui_hover_state.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/engine/ui/state/ui_state_factory.h"
#include "sunny_land/engine/ui/ui_interactive.h"

namespace pyc::sunny_land {

void UIHoverState::enter() {
    owner_->setSprite("hover");
    spdlog::debug("切换到悬停状态");
}

std::unique_ptr<UIState> UIHoverState::handleInput(Context& context) {
    auto& input_manager = context.getInputManager();
    auto mouse_pos = input_manager.getLogicalMousePosition();
    if (!owner_->isPointInside(mouse_pos)) {  // 如果鼠标不在UI元素内，则返回正常状态
        return UIStateFactory::create<UINormalState>(owner_);
    }
    if (input_manager.isActionPressed("MouseLeftClick")) {  // 如果鼠标按下，则返回按下状态
        return UIStateFactory::create<UIPressedState>(owner_);
    }
    return nullptr;
}

}  // namespace pyc::sunny_land