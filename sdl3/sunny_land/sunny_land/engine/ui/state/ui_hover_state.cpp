module;

#include <memory>

#include <SDL3/SDL_scancode.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include "common/string_hash.h"

module sunny_land.engine.ui.state;

import sunny_land.engine.core;
import sunny_land.engine.input.input_manager;
import sunny_land.engine.ui;

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
