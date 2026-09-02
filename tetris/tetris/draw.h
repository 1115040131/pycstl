#pragma once

#include <list>
#include <string_view>

#include "tetris/piece.h"
#include "tetris/play_field.h"
#include "tetris/tetromino.h"

namespace pyc {
namespace tetris {

enum class WindowStyle { kStyle1, kStyle2, kStyle3, kStyle4 };

template <WindowStyle style = WindowStyle::kStyle1>
void DrawWindow(int top, int left, int width, int height, std::string_view title);

template <std::size_t M, std::size_t N>
void DrawTetromino(const v1::Tetromino<M, N>& tetromino, int top, int left);

void DrawTetromino(const v2::Tetromino& tetromino, int top, int left, int index);

void DrawTetromino(const v3::TetrominoSet& tetromino_set, int top, int left, int index);

void DrawFrame(const Matrix& frame, int top, int left, bool clear_buffer);

void DrawPreview(const Matrix& preview, int top, int left);

void DrawHold(const Matrix& hold, int top, int left);

}  // namespace tetris
}  // namespace pyc
