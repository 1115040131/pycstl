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

void UIPressedState::enter() {
    owner_->setSprite("pressed");
    owner_->playSound("pressed");
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

}  // namespace pyc::sunny_land
