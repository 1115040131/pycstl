#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>

#include <SDL3_mixer/SDL_mixer.h>

#include "common/noncopyable.h"
#include "common/string_hash.h"

namespace pyc::sunny_land {

/**
 * @brief 管理 SDL_mixer 音效 (Mix_Chunk) 和音乐 (Mix_Music)。
 *
 * 提供音频资源的加载和缓存功能。构造失败时会抛出异常。
 * 仅供 ResourceManager 内部使用。
 */
class AudioManager final : Noncopyable {
    friend class ResourceManager;

public:
    /**
     * @brief 构造函数。初始化 SDL_mixer 并打开音频设备。
     * @throws std::runtime_error 如果 SDL_mixer 初始化或打开音频设备失败。
     */
    explicit AudioManager();

    ~AudioManager();  ///< @brief 需要手动添加析构函数，清理资源并关闭 SDL_mixer。

private:                                               // 仅供 ResourceManager 访问的方法
    Mix_Chunk* loadSound(std::string_view file_path);  ///< @brief 从文件路径加载音效
    Mix_Chunk* getSound(std::string_view file_path);   ///< @brief 尝试获取已加载音效的指针，如果未加载则尝试加载
    void unloadSound(std::string_view file_path);      ///< @brief 卸载指定的音效资源
    void clearSounds();                                ///< @brief 清空所有音效资源

    Mix_Music* loadMusic(std::string_view file_path);  ///< @brief 从文件路径加载音乐
    Mix_Music* getMusic(std::string_view file_path);   ///< @brief 尝试获取已加载音乐的指针，如果未加载则尝试加载
    void unloadMusic(std::string_view file_path);      ///< @brief 卸载指定的音乐资源
    void clearMusic();                                 ///< @brief 清空所有音乐资源

    void clearAudio();  ///< @brief 清空所有音频资源

private:
    // Mix_Chunk 的自定义删除器
    struct SDLMixChunkDeleter {
        void operator()(Mix_Chunk* chunk) const {
            if (chunk) {
                Mix_FreeChunk(chunk);
            }
        }
    };

    // Mix_Music 的自定义删除器
    struct SDLMixMusicDeleter {
        void operator()(Mix_Music* music) const {
            if (music) {
                Mix_FreeMusic(music);
            }
        }
    };

    // 音效存储 (文件路径 -> Mix_Chunk)
    std::unordered_map<std::string, std::unique_ptr<Mix_Chunk, SDLMixChunkDeleter>, StringHash, StringEqual>
        sounds_;
    // 音乐存储 (文件路径 -> Mix_Music)
    std::unordered_map<std::string, std::unique_ptr<Mix_Music, SDLMixMusicDeleter>, StringHash, StringEqual>
        music_;
};

}  // namespace pyc::sunny_land