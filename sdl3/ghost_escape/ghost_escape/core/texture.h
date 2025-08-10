#pragma once

#include <string>

#include <SDL3/SDL.h>

namespace pyc {
namespace sdl3 {

struct Texture {
    SDL_Texture* texture{};
    SDL_FRect src_rect{};
    float angle{};
    bool is_flip{};

    static Texture Create(const std::string& file_path);
};

}  // namespace sdl3
}  // namespace pyc
