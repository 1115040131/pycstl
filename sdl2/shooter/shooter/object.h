#pragma once

#include <chrono>

#include <SDL2/SDL.h>

namespace pyc {
namespace sdl2 {

using namespace std::chrono_literals;

template <typename Enum>
constexpr std::underlying_type_t<Enum> to_underlying(Enum e) noexcept {
    return static_cast<std::underlying_type_t<Enum>>(e);
}

struct Player {
    SDL_Texture* texture{};
    SDL_FPoint position{};
    int width{};
    int height{};
    int speed{300};
    int health{3};
    int max_health{3};
    std::chrono::duration<double> cool_down{0.3s};
    std::chrono::time_point<std::chrono::steady_clock> last_fire{};
};

struct Enemy {
    SDL_Texture* texture{};
    SDL_FPoint position{};
    int width{};
    int height{};
    int speed{150};
    int health{2};
    std::chrono::duration<double> cool_down{2s};
    std::chrono::time_point<std::chrono::steady_clock> last_fire{};
    bool valid{true};
};

struct Projectile {
    SDL_Texture* texture{};
    SDL_FPoint position{};
    SDL_FPoint direction{};
    int width{};
    int height{};
    int speed{400};
    int damage{1};
    bool valid{true};
};

struct Explosion {
    SDL_Texture* texture{};
    SDL_FPoint position{};
    int width{};
    int height{};
    int current_frame{};
    int total_frame{};
    std::chrono::time_point<std::chrono::steady_clock> start{};
    std::chrono::duration<double> frame_delay{100ms};
};

struct Item {
    enum class Type {
        kLife,
        kShield,
        kTime,
    };

    SDL_Texture* texture{};
    SDL_FPoint position{};
    SDL_FPoint direction{};
    int width{};
    int height{};
    int speed{200};
    int bounce_count{3};
    Type type{};
    bool valid{true};
};

struct Background {
    SDL_Texture* texture{};
    SDL_FPoint position{};
    float offset{};
    int width{};
    int height{};
    int speed{30};
};

}  // namespace sdl2
}  // namespace pyc