module;

#include <string>

#include <SDL3/SDL.h>

export module ghost_escape.core.texture;

export namespace pyc::sdl3 {

struct Texture {
    SDL_Texture* texture{};
    SDL_FRect src_rect{};
    float angle{};
    bool is_flip{};

    static Texture Create(const std::string& file_path);
};

}  // namespace pyc::sdl3
