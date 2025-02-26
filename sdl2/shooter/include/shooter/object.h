#pragma once

#include <chrono>

#include <SDL.h>

namespace pyc {
namespace sdl2 {

using namespace std::chrono_literals;

struct Player {
    SDL_Texture* texture{};
    SDL_FPoint position{};
    int width{};
    int height{};
    int speed{200};
    std::chrono::duration<double> cool_down{0.5s};
    std::chrono::time_point<std::chrono::steady_clock> last_fire{};
};

struct Enemy {
    SDL_Texture* texture{};
    SDL_FPoint position{};
    int width{};
    int height{};
    int speed{200};
};

struct Projectile {
    SDL_Texture* texture{};
    SDL_FPoint position{};
    int width{};
    int height{};
    int speed{400};
};

}  // namespace sdl2
}  // namespace pyc