#include "sdl3/common/affiliate/sprite.h"

#include <fmt/core.h>

namespace pyc {
namespace sdl3 {

Sprite* Sprite::CreateAndSet(ObjectScreen* parent, const std::string& file_path, float scale) {
    auto sprite = std::make_unique<Sprite>();
    sprite->init();
    sprite->setParent(parent);
    sprite->setTexture(Texture::Create(file_path));
    sprite->setScale(scale);
    return static_cast<Sprite*>(parent->addChild(std::move(sprite)));
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
    if (parent_->getType() == Object::Type::kCommon) {
        fmt::println("Sprite::render: parent is not ObjectScreen");
        return;
    }
    if (is_finish_) {
        return;
    }
    game_.renderTexture(texture_, static_cast<ObjectScreen*>(parent_)->getRenderPosition() + offset_, size_);
}

}  // namespace sdl3
}  // namespace pyc
