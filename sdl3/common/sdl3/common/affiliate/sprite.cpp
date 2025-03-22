#include "sdl3/common/affiliate/sprite.h"

#include <fmt/core.h>

namespace pyc {
namespace sdl3 {

Sprite* Sprite::Create(ObjectScreen* parent, const std::string& file_path, float scale) {
    auto sprite = std::make_unique<Sprite>();
    auto sprite_ptr = sprite.get();
    sprite->init();
    sprite->setParent(parent);
    sprite->setTexture(Texture::Create(file_path));
    sprite->setScale(scale);
    parent->addChild(std::move(sprite));
    return sprite_ptr;
}

void Sprite::setTexture(const Texture& texture) {
    texture_ = texture;
    size_ = glm::vec2(texture.src_rect.w, texture.src_rect.h);
}

void Sprite::render() {
    if (!texture_.texture || !parent_) {
        fmt::println("Sprite::render: texture or parent is nullptr");
        return;
    }
    game_.renderTexture(texture_, parent_->getRenderPosition() + offset_, size_);
}

}  // namespace sdl3
}  // namespace pyc
