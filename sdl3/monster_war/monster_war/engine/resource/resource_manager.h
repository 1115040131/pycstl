#pragma once

#include <memory>
#include <string_view>

#include <entt/core/fwd.hpp>
#include <glm/glm.hpp>

#include "common/noncopyable.h"

struct SDL_Renderer;
struct SDL_Texture;
struct Mix_Chunk;
struct Mix_Music;
struct TTF_Font;

namespace pyc::monster_war {

class TextureManager;
class AudioManager;
class FontManager;

/**
 * @brief 作为访问各种资源管理器的中央控制点（外观模式 Facade）。
 * 在构造时初始化其管理的子系统。构造失败会抛出异常。
 */
class ResourceManager final : Noncopyable {
public:
    /**
     * @brief 构造函数，执行初始化。
     * @param sdl_renderer SDL_Renderer 的指针，传递给需要它的子管理器。不能为空。
     */
    explicit ResourceManager(SDL_Renderer* sdl_renderer);

    ~ResourceManager();  // 显式声明析构函数，这是为了能让智能指针正确管理仅有前向声明的类

    void clear();  ///< @brief 清空所有资源

    // --- 统一资源访问接口 ---
    // -- Texture --
    SDL_Texture* loadTexture(entt::id_type id, std::string_view file_path);         ///< @brief 载入纹理资源(通过id + 文件路径)
    SDL_Texture* loadTexture(entt::hashed_string str_hs);                           ///< @brief 载入纹理资源(通过字符串哈希值)
    SDL_Texture* getTexture(entt::id_type id, std::string_view file_path = "");     ///< @brief 尝试获取已加载纹理的指针，如果未加载则尝试加载(通过id + 文件路径)
    SDL_Texture* getTexture(entt::hashed_string str_hs);                            ///< @brief 尝试获取已加载纹理的指针，如果未加载则尝试加载(通过字符串哈希值)
    void unloadTexture(entt::id_type id);                                           ///< @brief 卸载指定的纹理资源
    glm::vec2 getTextureSize(entt::id_type id, std::string_view file_path = "");    ///< @brief 获取指定纹理的尺寸(通过id + 文件路径)
    glm::vec2 getTextureSize(entt::hashed_string str_hs);                           ///< @brief 获取指定纹理的尺寸(通过字符串哈希值)
    void clearTextures();                                                           ///< @brief 清空所有纹理资源

    // -- Sound Effects (Chunks) --
    Mix_Chunk* loadSound(entt::id_type id, std::string_view file_path);             ///< @brief 载入音效资源(通过id + 文件路径)
    Mix_Chunk* loadSound(entt::hashed_string str_hs);                               ///< @brief 载入音效资源(通过字符串哈希值)
    Mix_Chunk* getSound(entt::id_type id, std::string_view file_path = "");         ///< @brief 尝试获取已加载音效的指针，如果未加载则尝试加载(通过id + 文件路径)
    Mix_Chunk* getSound(entt::hashed_string str_hs);                                ///< @brief 尝试获取已加载音效的指针，如果未加载则尝试加载(通过字符串哈希值)
    void unloadSound(entt::id_type id);                                             ///< @brief 卸载指定的音效资源
    void clearSounds();                                                             ///< @brief 清空所有音效资源

    // -- Music --
    Mix_Music* loadMusic(entt::id_type id, std::string_view file_path);             ///< @brief 载入音乐资源(通过id + 文件路径)
    Mix_Music* loadMusic(entt::hashed_string str_hs);                               ///< @brief 载入音乐资源(通过字符串哈希值)
    Mix_Music* getMusic(entt::id_type id, std::string_view file_path = "");         ///< @brief 尝试获取已加载音乐的指针，如果未加载则尝试加载(通过id + 文件路径)
    Mix_Music* getMusic(entt::hashed_string str_hs);                                ///< @brief 尝试获取已加载音乐的指针，如果未加载则尝试加载(通过字符串哈希值)
    void unloadMusic(entt::id_type id);                                             ///< @brief 卸载指定的音乐资源
    void clearMusic();                                                              ///< @brief 清空所有音乐资源

    // -- Fonts --
    TTF_Font* loadFont(entt::id_type id, int point_size, std::string_view file_path);     ///< @brief 载入字体资源(通过id + 文件路径)
    TTF_Font* loadFont(entt::hashed_string str_hs, int point_size);                       ///< @brief 载入字体资源(通过字符串哈希值)
    TTF_Font* getFont(entt::id_type id, int point_size, std::string_view file_path = ""); ///< @brief 尝试获取已加载字体的指针，如果未加载则尝试加载(通过id + 文件路径)
    TTF_Font* getFont(entt::hashed_string str_hs, int point_size);                        ///< @brief 尝试获取已加载字体的指针，如果未加载则尝试加载(通过字符串哈希值)
    void unloadFont(entt::id_type id, int point_size);                              ///< @brief 卸载指定的字体资源
    void clearFonts();                                                              ///< @brief 清空所有字体资源

private:
    std::unique_ptr<TextureManager> texture_manager_;
    std::unique_ptr<AudioManager> audio_manager_;
    std::unique_ptr<FontManager> font_manager_;
};

}  // namespace pyc::monster_war
