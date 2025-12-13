#include "monster_war/engine/ui/state/ui_normal_state.h"

#include <entt/core/hashed_string.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/core/context.h"
#include "monster_war/engine/input/input_manager.h"
#include "monster_war/engine/ui/state/ui_state_factory.h"
#include "monster_war/engine/ui/ui_interactive.h"

namespace pyc::monster_war {

using namespace entt::literals;

void UINormalState::enter() {
    owner_->setImage("normal"_hs);
    spdlog::debug("切换到正常状态");
}

std::unique_ptr<UIState> UINormalState::handleInput(Context& context) {
    auto& input_manager = context.getInputManager();
    auto mouse_pos = input_manager.getLogicalMousePosition();
    if (owner_->isPointInside(mouse_pos)) {  // 如果鼠标在UI元素内，则切换到悬停状态
        owner_->playSound("hover"_hs);
        return UIStateFactory::create<UIHoverState>(owner_);
    }
    return nullptr;
}

}  // namespace pyc::monster_war