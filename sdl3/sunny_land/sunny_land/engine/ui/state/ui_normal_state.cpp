module;

#include <memory>

#include <SDL3/SDL_scancode.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

module sunny_land.engine.ui.state;

import common.string_hash;

import sunny_land.engine.core;
import sunny_land.engine.input.input_manager;
import sunny_land.engine.ui;

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
