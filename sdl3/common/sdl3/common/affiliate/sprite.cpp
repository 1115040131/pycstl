#include "sdl3/common/affiliate/sprite.h"

#include <fmt/core.h>

namespace pyc {
namespace sdl3 {

Texture Texture::makeTexture(const std::string& file_path) {
    Texture texture{Game::GetInstance().getAssetStore()->getImage(file_path)};
    SDL_GetTextureSize(texture.texture, &texture.src_rect.w, &texture.src_rect.h);
    return texture;
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
