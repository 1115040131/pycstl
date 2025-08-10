#include "ghost_escape/core/asset_store.h"

#include <fmt/format.h>

namespace pyc {
namespace sdl3 {

AssetStore::~AssetStore() {
    for (auto& [_, texture] : textures_) {
        SDL_DestroyTexture(texture);
    }
    textures_.clear();

    for (auto& [_, sound] : sounds_) {
        Mix_FreeChunk(sound);
    }
    sounds_.clear();

    for (auto& [_, music] : music_) {
        Mix_FreeMusic(music);
    }
    music_.clear();

    for (auto& [_, font] : fonts_) {
        TTF_CloseFont(font);
    }
    fonts_.clear();
}

SDL_Texture* AssetStore::getImage(const std::string& file_path) {
    auto iter = textures_.find(file_path);
    if (iter == textures_.end()) {
        loadImage(file_path);
        iter = textures_.find(file_path);
        if (iter == textures_.end()) {
            fmt::println("Image not loaded: {}", file_path);
            return nullptr;
        }
    }
    return iter->second;
}

Mix_Chunk* AssetStore::getSound(const std::string& file_path) {
    auto iter = sounds_.find(file_path);
    if (iter == sounds_.end()) {
        loadSound(file_path);
        iter = sounds_.find(file_path);
        if (iter == sounds_.end()) {
            fmt::println("Sound not loaded: {}", file_path);
            return nullptr;
        }
    }
    return iter->second;
}

Mix_Music* AssetStore::getMusic(const std::string& file_path) {
    auto iter = music_.find(file_path);
    if (iter == music_.end()) {
        loadMusic(file_path);
        iter = music_.find(file_path);
        if (iter == music_.end()) {
            fmt::println("Music not loaded: {}", file_path);
            return nullptr;
        }
    }
    return iter->second;
}

TTF_Font* AssetStore::getFont(const std::string& file_path, int font_size) {
    auto key = fmt::format("{}:{}", file_path, font_size);

    auto iter = fonts_.find(key);
    if (iter == fonts_.end()) {
        loadFont(file_path, font_size);
        iter = fonts_.find(key);
        if (iter == fonts_.end()) {
            fmt::println("Font not loaded: {}", key);
            return nullptr;
        }
    }
    return iter->second;
}

void AssetStore::loadImage(const std::string& file_path) {
    if (textures_.contains(file_path)) {
        fmt::println("Image already loaded: {}", file_path);
        return;
    }

    SDL_Texture* texture = IMG_LoadTexture(renderer_, file_path.c_str());
    if (!texture) {
        fmt::println("Failed to load image: {}", file_path);
        return;
    }

    textures_[file_path] = texture;
}

void AssetStore::loadSound(const std::string& file_path) {
    if (sounds_.contains(file_path)) {
        fmt::println("Sound already loaded: {}", file_path);
        return;
    }

    Mix_Chunk* sound = Mix_LoadWAV(file_path.c_str());
    if (!sound) {
        fmt::println("Failed to load sound: {}", file_path);
        return;
    }

    sounds_[file_path] = sound;
}

void AssetStore::loadMusic(const std::string& file_path) {
    if (music_.contains(file_path)) {
        fmt::println("Music already loaded: {}", file_path);
        return;
    }

    Mix_Music* music = Mix_LoadMUS(file_path.c_str());
    if (!music) {
        fmt::println("Failed to load music: {}", file_path);
        return;
    }

    music_[file_path] = music;
}

void AssetStore::loadFont(const std::string& file_path, int font_size) {
    auto key = fmt::format("{}:{}", file_path, font_size);

    if (fonts_.contains(key)) {
        fmt::println("Font already loaded: {}", key);
        return;
    }

    TTF_Font* font = TTF_OpenFont(file_path.c_str(), font_size);
    if (!font) {
        fmt::println("Failed to load font: {}", file_path);
        return;
    }

    fonts_[key] = font;
}

}  // namespace sdl3
}  // namespace pyc
