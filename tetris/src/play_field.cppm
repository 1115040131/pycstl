module;

#include <cstddef>
#include <vector>

export module tetris.play_field;

export namespace pyc::tetris {

inline constexpr std::size_t kPlayFieldRow = 22;
inline constexpr std::size_t kPlayFieldCol = 10;

inline constexpr std::size_t kPreviewRow = 15;
inline constexpr std::size_t kPreviewCol = 6;

inline constexpr std::size_t kHoldRow = 4;
inline constexpr std::size_t kHoldCol = 7;

using Matrix = std::vector<std::vector<int>>;

}  // namespace pyc::tetris
