module;

#include <string>

#include <SDL3/SDL.h>

module ghost_escape.core.texture;

import ghost_escape.core;

namespace pyc::sdl3 {

Texture Texture::Create(const std::string& file_path) {
    Texture texture{Game::GetInstance().getAssetStore()->getImage(file_path)};
    SDL_GetTextureSize(texture.texture, &texture.src_rect.w, &texture.src_rect.h);
    return texture;
}

}  // namespace pyc::sdl3
