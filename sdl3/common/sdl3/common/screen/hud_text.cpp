#include "sdl3/common/screen/hud_text.h"

namespace pyc {
namespace sdl3 {

HUDText* HUDText::CreateAndSet(Object* parent, std::string_view text, const glm::vec2& render_position,
                               const glm::vec2& size, const std::string& font_path, int font_size,
                               const std::string& file_path, Anchor anchor) {
    auto hud_text = std::make_unique<HUDText>();
    hud_text->init();
#ifdef DEBUG_MODE
    hud_text->SET_NAME(HUDText);
#endif
    hud_text->sprite_bg_ = Sprite::CreateAndSet(hud_text.get(), file_path, 1.0f, anchor);
    hud_text->text_label_ = TextLabel::CreateAndSet(hud_text.get(), text, font_path, font_size, anchor);
    hud_text->setRenderPosition(render_position);
    hud_text->setSize(size);
    return static_cast<HUDText*>(parent->addChild(std::move(hud_text)));
}

void HUDText::setText(std::string_view text) {
    if (text_label_) {
        text_label_->setText(text);
    }
}

void HUDText::setSize(const glm::vec2& size) {
    size_ = size;
    if (sprite_bg_) {
        sprite_bg_->setSize(size);
    }
}

void HUDText::setBackground(const std::string& file_path) {
    if (sprite_bg_) {
        sprite_bg_->setTexture(Texture::Create(file_path));
    } else {
        sprite_bg_ = Sprite::CreateAndSet(this, file_path);
    }
}

}  // namespace sdl3
}  // namespace pyc