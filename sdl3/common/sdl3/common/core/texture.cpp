#include "sdl3/common/core/texture.h"

#include "sdl3/common/core/game.h"

namespace pyc {
namespace sdl3 {

Texture Texture::makeTexture(const std::string& file_path) {
    Texture texture{Game::GetInstance().getAssetStore()->getImage(file_path)};
    SDL_GetTextureSize(texture.texture, &texture.src_rect.w, &texture.src_rect.h);
    return texture;
}

}  // namespace sdl3
}  // namespace pyc