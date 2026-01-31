#include "monster_war/engine/ui/state/ui_hover_state.h"

#include <entt/core/hashed_string.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/core/context.h"
#include "monster_war/engine/input/input_manager.h"
#include "monster_war/engine/ui/state/ui_state_factory.h"
#include "monster_war/engine/ui/ui_interactive.h"

namespace pyc::monster_war {

using namespace entt::literals;

UIHoverState::UIHoverState(UIInteractive* owner) : UIState(owner) {
    owner_->getContext().getInputManager().onAction("mouse_left"_hs).connect<&UIHoverState::onMousePressed>(this);
}

UIHoverState::~UIHoverState() {
    owner_->getContext()
        .getInputManager()
        .onAction("mouse_left"_hs)
        .disconnect<&UIHoverState::onMousePressed>(this);
}

void UIHoverState::enter() {
    owner_->setCurrentImage("hover"_hs);
    owner_->hover_enter();
    spdlog::debug("切换到悬停状态");
}

void UIHoverState::update(std::chrono::duration<float>, Context& context) {
    auto& input_manager = context.getInputManager();
    auto mouse_pos = input_manager.getLogicalMousePosition();
    if (!owner_->isPointInside(mouse_pos)) {  // 如果鼠标不在UI元素内，则返回正常状态
        owner_->hover_leave();
        owner_->setNextState(UIStateFactory::create<UINormalState>(owner_));
    }
}

bool UIHoverState::onMousePressed() {
    owner_->setNextState(UIStateFactory::create<UIPressedState>(owner_));
    return true;
}

}  // namespace pyc::monster_war