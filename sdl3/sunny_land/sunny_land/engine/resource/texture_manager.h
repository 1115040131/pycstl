#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>

#include <SDL3/SDL_render.h>
#include <glm/glm.hpp>

#include "common/noncopyable.h"
#include "common/string_hash.h"

namespace pyc::sunny_land {

/**
 * @brief 管理 SDL_Texture 资源的加载、存储和检索。
 *
 * 在构造时初始化。使用文件路径作为键，确保纹理只加载一次并正确释放。
 * 依赖于一个有效的 SDL_Renderer，构造失败会抛出异常。
 */
class TextureManager final : Noncopyable {
    friend class ResourceManager;

public:
    /**
     * @brief 构造函数，执行初始化。
     * @param renderer 指向有效的 SDL_Renderer 上下文的指针。不能为空。
     * @throws std::runtime_error 如果 renderer 为 nullptr 或初始化失败。
     */
    explicit TextureManager(SDL_Renderer* renderer);

private:                                                   // 仅供 ResourceManager 访问的方法
    SDL_Texture* loadTexture(std::string_view file_path);  ///< @brief 从文件路径加载纹理
    SDL_Texture* getTexture(
        std::string_view file_path);  ///< @brief 尝试获取已加载纹理的指针，如果未加载则尝试加载
    glm::vec2 getTextureSize(std::string_view file_path);  ///< @brief 获取指定纹理的尺寸
    void unloadTexture(std::string_view file_path);        ///< @brief 卸载指定的纹理资源
    void clearTextures();                                  ///< @brief 清空所有纹理资源

private:
    struct SDLTextureDeleter {
        void operator()(SDL_Texture* texture) const {
            if (texture) {
                SDL_DestroyTexture(texture);
            }
        }
    };

    std::unordered_map<std::string, std::unique_ptr<SDL_Texture, SDLTextureDeleter>, StringHash, StringEqual>
        textures_;

    SDL_Renderer* renderer_ = nullptr;  // 指向主渲染器的非拥有指针
};

}  // namespace pyc::sunny_land