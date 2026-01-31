#include "monster_war/engine/ui/ui_panel.h"

#include <spdlog/spdlog.h>

#include "monster_war/engine/core/context.h"
#include "monster_war/engine/render/renderer.h"

namespace pyc::monster_war {

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

}  // namespace pyc::monster_war