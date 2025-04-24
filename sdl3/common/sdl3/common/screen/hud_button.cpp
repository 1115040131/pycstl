#include "sdl3/common/screen/hud_button.h"

namespace pyc {
namespace sdl3 {

HUDButton* HUDButton::CreateAndSet(Object* parent, const glm::vec2 render_position, const std::string& file_normal,
                                   const std::string& file_hover, const std::string& file_press, float scale,
                                   Anchor anchor) {
    auto hud_button = std::make_unique<HUDButton>();
    hud_button->init();
#ifdef DEBUG_MODE
    hud_button->SET_NAME(HUDButton);
#endif
    hud_button->setRenderPosition(render_position);
    hud_button->sprite_normal_ = Sprite::CreateAndSet(hud_button.get(), file_normal, scale, anchor);
    hud_button->sprite_hover_ = Sprite::CreateAndSet(hud_button.get(), file_hover, scale, anchor);
    hud_button->sprite_press_ = Sprite::CreateAndSet(hud_button.get(), file_press, scale, anchor);
    hud_button->sprite_hover_->setActive(false);
    hud_button->sprite_press_->setActive(false);
    return static_cast<HUDButton*>(parent->addChild(std::move(hud_button)));
}

bool HUDButton::handleEvents(const SDL_Event& event) {
    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            if (is_hover_) {
                is_press_ = true;
                game_.playSound(ASSET("sound/UI_button08.wav"));
                return true;
            }
        }
    } else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        if (event.button.button == SDL_BUTTON_LEFT) {
            is_press_ = false;
            if (is_hover_) {
                is_trigger_ = true;
                return true;
            }
        }
    }
    return false;
}

void HUDButton::update(std::chrono::duration<float>) {
    checkHover();
    checkState();
}

bool HUDButton::getIsTrigger() {
    if (is_trigger_) {
        is_hover_ = false;
        is_press_ = false;
        is_trigger_ = false;
        return true;
    }
    return false;
}

void HUDButton::checkHover() {
    bool prev_hover = is_hover_;

    const auto& position = render_position_ + sprite_normal_->getOffset();
    const auto& size = sprite_normal_->getSize();
    is_hover_ = game_.isMouseInRect(position, position + size);

    if (!prev_hover && is_hover_ && !is_press_) {
        game_.playSound(ASSET("sound/UI_button12.wav"));
    }
}

void HUDButton::checkState() {
    if (is_press_) {
        sprite_normal_->setActive(false);
        sprite_hover_->setActive(false);
        sprite_press_->setActive(true);
    } else if (is_hover_) {
        sprite_normal_->setActive(false);
        sprite_hover_->setActive(true);
        sprite_press_->setActive(false);
    } else {
        sprite_normal_->setActive(true);
        sprite_hover_->setActive(false);
        sprite_press_->setActive(false);
    }
}

}  // namespace sdl3
}  // namespace pyc
