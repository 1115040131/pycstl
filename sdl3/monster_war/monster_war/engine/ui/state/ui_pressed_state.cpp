#include "monster_war/engine/ui/state/ui_pressed_state.h"

#include <entt/core/hashed_string.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/core/context.h"
#include "monster_war/engine/input/input_manager.h"
#include "monster_war/engine/ui/state/ui_state_factory.h"
#include "monster_war/engine/ui/ui_interactive.h"

namespace pyc::monster_war {

using namespace entt::literals;

void UIPressedState::enter() {
    owner_->setSprite("pressed"_hs);
    owner_->playSound("pressed"_hs);
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

}  // namespace pyc::monster_war