#include "sunny_land/engine/resource/texture_manager.h"

#include <SDL3_image/SDL_image.h>
#include <spdlog/spdlog.h>

namespace pyc::sunny_land {

TextureManager::TextureManager(SDL_Renderer* sdl_renderer) : sdl_renderer_(sdl_renderer) {
    if (!sdl_renderer_) {
        throw std::runtime_error("TextureManager 构造失败: 渲染器指针为空。");
    }
    // SDL3中不再需要手动调用IMG_Init/IMG_Quit
    spdlog::trace("TextureManager 构造成功。");
}

SDL_Texture* TextureManager::loadTexture(std::string_view file_path) {
    auto it = textures_.find(file_path);
    if (it != textures_.end()) {
        return it->second.get();
    }

    // 如果没加载则尝试加载纹理
    auto raw_texture = IMG_LoadTexture(sdl_renderer_, file_path.data());

    // 载入纹理时，设置纹理缩放模式为最邻近插值(必不可少，否则TileLayer渲染中会出现边缘空隙/模糊)
    if (!SDL_SetTextureScaleMode(raw_texture, SDL_SCALEMODE_NEAREST)) {
        spdlog::warn("无法设置纹理缩放模式为最邻近插值");
    }

    if (!raw_texture) {
        spdlog::error("加载纹理失败: '{}': {}", file_path, SDL_GetError());
        return nullptr;
    }

    // 使用带有自定义删除器的 unique_ptr 存储加载的纹理
    textures_.emplace(file_path, std::unique_ptr<SDL_Texture, SDLTextureDeleter>(raw_texture));
    spdlog::debug("成功加载并缓存纹理: {}", file_path);

    return raw_texture;
}

SDL_Texture* TextureManager::getTexture(std::string_view file_path) {
    // 查找现有纹理
    auto it = textures_.find(file_path);
    if (it != textures_.end()) {
        return it->second.get();
    }

    // 如果未找到，尝试加载它
    spdlog::warn("纹理 '{}' 未找到缓存，尝试加载。", file_path);
    return loadTexture(file_path);
}

glm::vec2 TextureManager::getTextureSize(std::string_view file_path) {
    // 获取纹理
    auto texture = getTexture(file_path);
    if (!texture) {
        spdlog::error("无法获取纹理: {}", file_path);
        return glm::vec2(0);
    }

    // 获取纹理尺寸
    glm::vec2 size;
    if (!SDL_GetTextureSize(texture, &size.x, &size.y)) {
        spdlog::error("无法查询纹理尺寸: {}", file_path);
        return glm::vec2(0);
    }
    return size;
}

void TextureManager::unloadTexture(std::string_view file_path) {
    auto it = textures_.find(file_path);
    if (it != textures_.end()) {
        spdlog::debug("卸载纹理: {}", file_path);
        textures_.erase(it);  // unique_ptr 通过自定义删除器处理删除
    } else {
        spdlog::warn("尝试卸载不存在的纹理: {}", file_path);
    }
}

void TextureManager::clearTextures() {
    if (!textures_.empty()) {
        spdlog::debug("正在清除所有 {} 个缓存的纹理。", textures_.size());
        textures_.clear();  // unique_ptr 处理所有元素的删除
    }
}

}  // namespace pyc::sunny_land