export module tetris.utils;

export import tetris.piece;
export import tetris.play_field;

export namespace pyc::tetris {

inline constexpr int Block2Col(int block_idx) { return 2 * block_idx - 1; }

// 将块合并到 play_field_ 中
void Merge(Matrix& play_field, const Piece& piece);

}  // namespace pyc::tetris
