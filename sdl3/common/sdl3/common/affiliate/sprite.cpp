#include "sdl3/common/affiliate/sprite.h"

#include "sdl3/common/core/object_screen.h"

namespace pyc {
namespace sdl3 {

Sprite* Sprite::CreateAndSet(ObjectScreen* parent, const std::string& file_path, float scale, Anchor anchor) {
    auto sprite = std::make_unique<Sprite>();
    sprite->init();
#ifdef DEBUG_MODE
    sprite->SET_NAME(Sprite);
#endif
    sprite->setTexture(Texture::Create(file_path));
    sprite->setScale(scale);
    sprite->setOffsetByAnchor(anchor);
    return static_cast<Sprite*>(parent->addChild(std::move(sprite)));
}

void Sprite::setTexture(const Texture& texture) {
    texture_ = texture;
    size_ = glm::vec2(texture.src_rect.w, texture.src_rect.h);
}

void Sprite::render() {
    if (!texture_.texture || !parent_) {
        // fmt::println("Sprite::render: texture or parent is nullptr");
        return;
    }
    if (is_finish_) {
        return;
    }
    game_.renderTexture(texture_, getRenderPosition(), size_, percent_);
}

}  // namespace sdl3
}  // namespace pyc
