#pragma once

#include <string_view>
#include <vector>

#include <fmt/format.h>

#include <SDL.h>

namespace pyc {
namespace sdl2 {

class Atlas {
public:
    Atlas() = default;

    ~Atlas();

    /// @brief 从磁盘上加载一组图片
    void load(SDL_Renderer* renderer, fmt::format_string<int> path_template, int num);

    /// @brief 清空所有的纹理
    void clear();

    /// @brief 获取纹理的数量
    size_t size() const;

    /// @brief 获取指定索引的纹理
    SDL_Texture* get_texture(size_t index) const;

    /// @brief 添加一个纹理
    void add_texture(SDL_Texture* texture);

private:
    std::vector<SDL_Texture*> textures_;
};

}  // namespace sdl2
}  // namespace pyc
