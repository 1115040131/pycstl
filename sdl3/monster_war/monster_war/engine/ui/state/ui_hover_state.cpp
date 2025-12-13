#include "monster_war/engine/ui/state/ui_hover_state.h"

#include <entt/core/hashed_string.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/core/context.h"
#include "monster_war/engine/input/input_manager.h"
#include "monster_war/engine/ui/state/ui_state_factory.h"
#include "monster_war/engine/ui/ui_interactive.h"

namespace pyc::monster_war {

using namespace entt::literals;

void UIHoverState::enter() {
    owner_->setImage("hover"_hs);
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

}  // namespace pyc::monster_war