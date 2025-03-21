#include "sdl3/common/affiliate/sprite.h"

#include <fmt/core.h>

namespace pyc {
namespace sdl3 {

std::shared_ptr<Sprite> Sprite::Create(ObjectScreen* parent, const std::string& file_path, float scale) {
    auto sprite = std::make_shared<Sprite>();
    sprite->init();
    sprite->setParent(parent);
    sprite->setTexture(Texture::Create(file_path));
    sprite->setScale(scale);
    return sprite;
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
