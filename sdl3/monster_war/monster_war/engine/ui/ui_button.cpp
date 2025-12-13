#include "monster_war/engine/ui/ui_button.h"

#include <entt/core/hashed_string.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/render/image.h"
#include "monster_war/engine/ui/state/ui_state_factory.h"

namespace pyc::monster_war {

using namespace entt::literals;

UIButton::UIButton(Context& context, std::string_view normal_image_id, std::string_view hover_image_id,
                   std::string_view pressed_image_id, glm::vec2 position, glm::vec2 size,
                   std::function<void()> callback)
    : UIInteractive(context, std::move(position), std::move(size)), callback_(std::move(callback)) {
    addImage("normal"_hs, Image(normal_image_id));
    addImage("hover"_hs, Image(hover_image_id));
    addImage("pressed"_hs, Image(pressed_image_id));

    // 设置默认状态为"normal"
    setState(UIStateFactory::create<UINormalState>(this));

    // 设置默认音效
    addSound("hover"_hs, "assets/audio/button_hover.wav"_hs);
    addSound("pressed"_hs, "assets/audio/button_click.wav"_hs);
    spdlog::trace("UIButton 构造完成");
}

void UIButton::clicked() {
    if (callback_) {
        callback_();
    }
}

}  // namespace pyc::monster_war