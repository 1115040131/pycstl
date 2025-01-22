#include "sdl2/common/atlas.h"

#include <SDL_image.h>
#include <fmt/format.h>

namespace pyc {
namespace sdl2 {

Atlas::~Atlas() { clear(); }

void Atlas::load(SDL_Renderer* renderer, fmt::format_string<int> path_template, int num) {
    for (int i = 0; i < num; i++) {
        SDL_Texture* texture =
            IMG_LoadTexture(renderer, fmt::format(path_template, std::forward<int>(i + 1)).c_str());
        textures_.push_back(texture);
    }
}

void Atlas::clear() {
    for (auto texture : textures_) {
        SDL_DestroyTexture(texture);
    }
    textures_.clear();
}

size_t Atlas::size() const { return textures_.size(); }

SDL_Texture* Atlas::get_texture(size_t index) const {
    if (index >= textures_.size()) {
        return nullptr;
    }
    return textures_[index];
}

void Atlas::add_texture(SDL_Texture* texture) { textures_.push_back(texture); }

}  // namespace sdl2
}  // namespace pyc