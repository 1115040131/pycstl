#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>

#include <SDL3_ttf/SDL_ttf.h>
#include <entt/core/fwd.hpp>

#include "common/noncopyable.h"

namespace pyc::monster_war {

// 定义字体键类型（路径 + 大小）
struct FontKey {
    entt::id_type id;
    int size;

    constexpr auto operator<=>(const FontKey&) const = default;
};

/**
 * @brief FontKey 的自定义哈希函数，适用于 std::unordered_map。
 *        使用标准库推荐的哈希合并方式，避免简单异或带来的哈希冲突。
 */
struct FontKeyHash {
    std::size_t operator()(const FontKey& key) const noexcept {
        // 采用C++20标准库的hash_combine实现思路
        std::size_t h1 = std::hash<entt::id_type>{}(key.id);
        std::size_t h2 = std::hash<int>{}(key.size);
        // 使用标准库推荐的哈希合并方式
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

/**
 * @brief 管理 SDL_ttf 字体资源（TTF_Font）。
 *
 * 提供字体的加载和缓存功能，通过文件路径和点大小来标识。
 * 构造失败会抛出异常。仅供 ResourceManager 内部使用。
 */
class FontManager final : Noncopyable {
    friend class ResourceManager;

public:
    /**
     * @brief 构造函数。初始化 SDL_ttf。
     * @throws std::runtime_error 如果 SDL_ttf 初始化失败。
     */
    explicit FontManager();

    ~FontManager();  ///< @brief 需要手动添加析构函数，清理资源并关闭 SDL_ttf。

private:  // 仅由 ResourceManager（和内部）访问的方法
    /**
     * @brief 从文件路径加载指定点大小的字体
     * @param id 字体的唯一标识符, 通过entt::hashed_string生成
     * @param point_size 字体的点大小
     * @param file_path 字体文件的路径
     * @return 加载的字体的指针
     * @note 如果字体已经加载，则返回已加载字体的指针
     * @note 如果字体未加载，则从文件路径加载字体，并返回加载的字体的指针
     */
    TTF_Font* loadFont(entt::id_type id, int point_size, std::string_view file_path);

    /**
     * @brief 从字符串哈希值加载指定点大小的字体
     * @param str_hs entt::hashed_string类型
     * @param point_size 字体的点大小
     * @return 加载的字体的指针
     * @note 如果字体已经加载，则返回已加载字体的指针
     * @note 如果字体未加载，则从哈希字符串对应的文件路径加载字体，并返回加载的字体的指针
     */
    TTF_Font* loadFont(entt::hashed_string str_hs, int point_size);

    /**
     * @brief 尝试获取已加载字体的指针，如果未加载则尝试加载
     * @param id 字体的唯一标识符, 通过entt::hashed_string生成
     * @param point_size 字体的点大小
     * @param file_path 字体文件的路径
     * @return 加载的字体的指针
     * @note 如果字体已经加载，则返回已加载字体的指针
     * @note 如果字体未加载，且提供了file_path，则尝试从文件路径加载字体，并返回加载的字体的指针
     */
    TTF_Font* getFont(entt::id_type id, int point_size, std::string_view file_path = "");

    /**
     * @brief 从字符串哈希值获取字体
     * @param str_hs entt::hashed_string类型
     * @param point_size 字体的点大小
     * @return 加载的字体的指针
     * @note 如果字体已经加载，则返回已加载字体的指针
     * @note 如果字体未加载，则从哈希字符串对应的文件路径加载字体，并返回加载的字体的指针
     */
    TTF_Font* getFont(entt::hashed_string str_hs, int point_size);

    /**
     * @brief 卸载特定字体（通过路径哈希值和大小标识）
     * @param id 字体的唯一标识符, 通过entt::hashed_string生成
     * @param point_size 字体的点大小
     */
    void unloadFont(entt::id_type id, int point_size);

    /**
     * @brief 清空所有缓存的字体
     */
    void clearFonts();

private:
    // TTF_Font 的自定义删除器
    struct SDLFontDeleter {
        void operator()(TTF_Font* font) const {
            if (font) {
                TTF_CloseFont(font);
            }
        }
    };

    // 字体存储（FontKey -> TTF_Font）
    std::unordered_map<FontKey, std::unique_ptr<TTF_Font, SDLFontDeleter>, FontKeyHash> fonts_;
};

}  // namespace pyc::monster_war