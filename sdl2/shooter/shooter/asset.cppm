module;

#include <array>
#include <cstddef>
#include <string_view>

export module sdl2.shooter.asset;

export namespace pyc::sdl2 {

inline constexpr std::string_view kAssetPath = "sdl2/shooter/assets/";

// 前缀与文件名在编译期拼好，落成一个只读字符数组
template <std::size_t N>
struct AssetPath {
    std::array<char, N> chars{};

    constexpr operator const char*() const { return chars.data(); }
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

}  // namespace pyc::sdl2
