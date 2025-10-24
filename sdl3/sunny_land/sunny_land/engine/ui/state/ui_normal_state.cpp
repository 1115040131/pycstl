#include "sunny_land/engine/ui/state/ui_normal_state.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/engine/ui/state/ui_state_factory.h"
#include "sunny_land/engine/ui/ui_interactive.h"

namespace pyc::sunny_land {

void UINormalState::enter() {
    owner_->setSprite("normal");
    spdlog::debug("切换到正常状态");
}

std::unique_ptr<UIState> UINormalState::handleInput(Context& context) {
    auto& input_manager = context.getInputManager();
    auto mouse_pos = input_manager.getLogicalMousePosition();
    if (owner_->isPointInside(mouse_pos)) {  // 如果鼠标在UI元素内，则切换到悬停状态
        owner_->playSound("hover");
        return UIStateFactory::create<UIHoverState>(owner_);
    }
    return nullptr;
}

}  // namespace pyc::sunny_land