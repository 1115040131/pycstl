module;

#include <entt/core/hashed_string.hpp>
#include <spdlog/spdlog.h>

module monster_war.engine.ui.state.ui_normal_state;

import monster_war.engine.core.context;
import monster_war.engine.input.input_manager;
import monster_war.engine.ui;
import monster_war.engine.ui.state.ui_hover_state;
import monster_war.engine.ui.state.ui_state_factory;

namespace pyc::monster_war {

using namespace entt::literals;

void UINormalState::enter() {
    owner_->setCurrentImage("normal"_hs);
    spdlog::debug("切换到正常状态");
}

void UINormalState::update(std::chrono::duration<float>, Context& context) {
    auto& input_manager = context.getInputManager();
    auto mouse_pos = input_manager.getLogicalMousePosition();
    if (owner_->isPointInside(mouse_pos)) {  // 如果鼠标在UI元素内，则切换到悬停状态
        owner_->playSound("ui_hover"_hs);
        owner_->setNextState(UIStateFactory::create<UIHoverState>(owner_));
    }
}

}  // namespace pyc::monster_war
