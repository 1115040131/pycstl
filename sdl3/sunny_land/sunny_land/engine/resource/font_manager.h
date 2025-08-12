#pragma once

#include <memory>
#include <string_view>
#include <unordered_map>

#include <SDL3_ttf/SDL_ttf.h>

#include "common/noncopyable.h"

namespace pyc::sunny_land {

// 定义字体键类型（路径 + 大小）
struct FontKey {
    std::string file_path;
    int size;
};

struct FontKeyLookup {
    std::string_view file_path;
    int size;
};

struct FontKeyHash {
    using is_transparent = void;  // 标记支持异构查找

    std::size_t operator()(const FontKey& key) const { return hash_impl(key.file_path, key.size); }
    std::size_t operator()(const FontKeyLookup& key) const { return hash_impl(key.file_path, key.size); }

private:
    std::size_t hash_impl(std::string_view path, int size) const noexcept {
        std::size_t h1 = std::hash<std::string_view>{}(path);
        std::size_t h2 = std::hash<int>{}(size);

        // 组合哈希值
        return h1 ^ h2;
    }
};

struct FontKeyEqual {
    using is_transparent = void;  // 标记支持异构查找

    // FontKey vs FontKey
    bool operator()(const FontKey& lhs, const FontKey& rhs) const noexcept {
        return lhs.size == rhs.size && lhs.file_path == rhs.file_path;
    }

    // FontKey vs FontKeyLookup
    bool operator()(const FontKey& lhs, const FontKeyLookup& rhs) const noexcept {
        return lhs.size == rhs.size && lhs.file_path == rhs.file_path;
    }

    // FontKeyLookup vs FontKey
    bool operator()(const FontKeyLookup& lhs, const FontKey& rhs) const noexcept {
        return lhs.size == rhs.size && lhs.file_path == rhs.file_path;
    }

    // FontKeyLookup vs FontKeyLookup
    bool operator()(const FontKeyLookup& lhs, const FontKeyLookup& rhs) const noexcept {
        return lhs.size == rhs.size && lhs.file_path == rhs.file_path;
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

private:                                                             // 仅由 ResourceManager（和内部）访问的方法
    TTF_Font* loadFont(std::string_view file_path, int point_size);  ///< @brief 从文件路径加载指定点大小的字体
    TTF_Font* getFont(std::string_view file_path,
                      int point_size);  ///< @brief 尝试获取已加载字体的指针，如果未加载则尝试加载
    void unloadFont(std::string_view file_path, int point_size);  ///< @brief 卸载特定字体（通过路径和大小标识）
    void clearFonts();                                            ///< @brief 清空所有缓存的字体

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
    std::unordered_map<FontKey, std::unique_ptr<TTF_Font, SDLFontDeleter>, FontKeyHash, FontKeyEqual> fonts_;
};

}  // namespace pyc::sunny_land