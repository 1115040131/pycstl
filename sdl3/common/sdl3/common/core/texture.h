#pragma once

#include <SDL3/SDL.h>
#include <string>

namespace pyc {
namespace sdl3 {

struct Texture {
    SDL_Texture* texture{};
    SDL_FRect src_rect{};
    float angle{};
    bool is_flip{};

    static Texture makeTexture(const std::string& file_path);
};

}  // namespace sdl3
}  // namespace pyc
