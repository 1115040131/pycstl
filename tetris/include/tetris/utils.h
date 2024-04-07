#pragma once

namespace pyc {
namespace tetris {

inline constexpr int Block2Col(int block_idx) { return 2 * block_idx - 1; }

}  // namespace tetris
}  // namespace pyc
