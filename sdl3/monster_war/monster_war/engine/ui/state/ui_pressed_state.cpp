#include "monster_war/engine/ui/state/ui_pressed_state.h"

#include <entt/core/hashed_string.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/core/context.h"
#include "monster_war/engine/input/input_manager.h"
#include "monster_war/engine/ui/state/ui_state_factory.h"
#include "monster_war/engine/ui/ui_interactive.h"

namespace pyc::monster_war {

using namespace entt::literals;

UIPressedState::UIPressedState(UIInteractive* owner) : UIState(owner) {
    owner_->getContext()
        .getInputManager()
        .onAction("mouse_left"_hs, ActionState::RELEASED)
        .connect<&UIPressedState::onMouseReleased>(this);
}

UIPressedState::~UIPressedState() {
    owner_->getContext()
        .getInputManager()
        .onAction("mouse_left"_hs, ActionState::RELEASED)
        .disconnect<&UIPressedState::onMouseReleased>(this);
}

void UIPressedState::enter() {
    owner_->setCurrentImage("pressed"_hs);
    owner_->playSound("ui_click"_hs);
    spdlog::debug("切换到按下状态");
}

bool UIPressedState::onMouseReleased() {
    auto& input_manager = owner_->getContext().getInputManager();
    auto mouse_pos = input_manager.getLogicalMousePosition();
    if (!owner_->isPointInside(mouse_pos)) {
        owner_->setNextState(UIStateFactory::create<UINormalState>(owner_));
    } else {
        owner_->clicked();
        owner_->setNextState(UIStateFactory::create<UIHoverState>(owner_));
    }
    return true;
}

}  // namespace pyc::monster_war