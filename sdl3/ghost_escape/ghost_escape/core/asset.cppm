module;

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

export module ghost_escape.core:asset;

export namespace pyc::sdl3 {

inline constexpr std::string_view kAssetPath = "sdl3/ghost_escape/assets/";

// 前缀与文件名在编译期拼好，落成一个只读字符数组
template <std::size_t N>
struct AssetPath {
    std::array<char, N> chars{};

    constexpr operator std::string_view() const { return {chars.data(), chars.size() - 1}; }
    operator std::string() const { return {chars.data(), chars.size() - 1}; }
};

template <std::size_t N>
consteval AssetPath<kAssetPath.size() + N> Asset(const char (&filename)[N]) {
    AssetPath<kAssetPath.size() + N> result{};
    for (std::size_t i = 0; i < kAssetPath.size(); ++i) {
        result.chars[i] = kAssetPath[i];
    }
    for (std::size_t i = 0; i < N; ++i) {
        result.chars[kAssetPath.size() + i] = filename[i];
    }
    return result;
}

}  // namespace pyc::sdl3
