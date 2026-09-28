module;

#include <optional>
#include <utility>

#include <SDL3/SDL_rect.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

module sunny_land.engine.ui.ui_panel;

import sunny_land.engine.core;
import sunny_land.engine.ui.ui_element;

namespace pyc::sunny_land {

UIPanel::UIPanel(glm::vec2 position, glm::vec2 size, std::optional<FColor> background_color)
    : UIElement(std::move(position), std::move(size)), background_color_(std::move(background_color)) {
    spdlog::trace("UIPanel 构造完成。");
}

void UIPanel::render(Context& context) {
    if (!visible_) {
        return;
    }

    if (background_color_) {
        context.getRenderer().drawUIFilledRect(getBounds(), background_color_.value());
    }

    UIElement::render(context);
}

}  // namespace pyc::sunny_land
