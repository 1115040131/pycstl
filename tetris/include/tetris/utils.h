#pragma once

#include "tetris/piece.h"
#include "tetris/play_field.h"

namespace pyc {
namespace tetris {

inline constexpr int Block2Col(int block_idx) { return 2 * block_idx - 1; }

// 将块合并到 play_field_ 中
void Merge(Matrix& play_field, const Piece& piece);

}  // namespace tetris
}  // namespace pyc
