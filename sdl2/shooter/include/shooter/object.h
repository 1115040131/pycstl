#pragma once

#include <SDL.h>

namespace pyc {
namespace sdl2 {

struct Player {
    SDL_Texture* texture{};
    SDL_FPoint position{};
    int width{};
    int height{};
    int speed{200};
};

}  // namespace sdl2
}  // namespace pyc