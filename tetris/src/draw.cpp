#include "tetris/draw.h"

#include <limits>
#include <sstream>
#include <unordered_map>

#include <fmt/core.h>

#include "tetris/terminal.h"
#include "tetris/utils.h"

namespace pyc {
namespace tetris {

// clang-format off
/*
     	0	1	2	3	4	5	6	7	8	9	A	B	C	D	E	F
U+250x	─	━	│	┃	┄	┅	┆	┇	┈	┉	┊	┋	┌	┍	┎	┏
U+251x	┐	┑	┒	┓	└	┕	┖	┗	┘	┙	┚	┛	├	┝	┞	┟
U+252x	┠	┡	┢	┣	┤	┥	┦	┧	┨	┩	┪	┫	┬	┭	┮	┯
U+253x	┰	┱	┲	┳	┴	┵	┶	┷	┸	┹	┺	┻	┼	┽	┾	┿
U+254x	╀	╁	╂	╃	╄	╅	╆	╇	╈	╉	╊	╋	╌	╍	╎	╏
U+255x	═	║	╒	╓	╔	╕	╖	╗	╘	╙	╚	╛	╜	╝	╞	╟
U+256x	╠	╡	╢	╣	╤	╥	╦	╧	╨	╩	╪	╫	╬	╭	╮	╯
U+257x	╰	╱	╲	╳	╴	╵	╶	╷	╸	╹	╺	╻	╼	╽	╾	╿
*/
// clang-format on

template <WindowStyle style>
void DrawWindow(int top, int left, int width, int height, std::string_view title) {
    if (width < 2 || height < 2) {
        fmt::println("Draw window error, width: {}, left: {}", width, height);
        return;
    }

    const auto& terminal = Terminal::GetInstance();

    int padding_width = 2 * width - 4;

    if constexpr (style == WindowStyle::kStyle1) {
        terminal.MoveTo(top, Block2Col(left)).Output(fmt::format(" ┌{:─^{}}┐ ", title, padding_width));
        for (int i = 1; i < height - 1; i++) {
            terminal.MoveTo(top + i, Block2Col(left)).Output(fmt::format(" │{:<{}}│ ", "", padding_width));
        }
        terminal.MoveTo(top + height - 1, Block2Col(left)).Output(fmt::format(" └{:─<{}}┘ ", "", padding_width));
    } else if constexpr (style == WindowStyle::kStyle2) {
        terminal.MoveTo(top, Block2Col(left)).Output(fmt::format(" ╔{:═^{}}╗ ", title, padding_width));
        for (int i = 1; i < height - 1; i++) {
            terminal.MoveTo(top + i, Block2Col(left)).Output(fmt::format(" ║{:<{}}║ ", "", padding_width));
        }
        terminal.MoveTo(top + height - 1, Block2Col(left)).Output(fmt::format(" ╚{:═<{}}╝ ", "", padding_width));
    } else if constexpr (style == WindowStyle::kStyle3) {
        terminal.MoveTo(top, Block2Col(left)).Output(fmt::format(" ┏{:━^{}}┓ ", title, padding_width));
        for (int i = 1; i < height - 1; i++) {
            terminal.MoveTo(top + i, Block2Col(left)).Output(fmt::format(" ┃{:<{}}┃ ", "", padding_width));
        }
        terminal.MoveTo(top + height - 1, Block2Col(left)).Output(fmt::format(" ┗{:━<{}}┛ ", "", padding_width));
    } else if constexpr (style == WindowStyle::kStyle4) {
        terminal.MoveTo(top, Block2Col(left)).Output(fmt::format(" ╭{:─^{}}╮ ", title, padding_width));
        for (int i = 1; i < height - 1; i++) {
            terminal.MoveTo(top + i, Block2Col(left)).Output(fmt::format(" │{:<{}}│ ", "", padding_width));
        }
        terminal.MoveTo(top + height - 1, Block2Col(left)).Output(fmt::format(" ╰{:─<{}}╯ ", "", padding_width));
    }
}

template void DrawWindow<WindowStyle::kStyle1>(int top, int left, int height, int width, std::string_view title);
template void DrawWindow<WindowStyle::kStyle2>(int top, int left, int height, int width, std::string_view title);
template void DrawWindow<WindowStyle::kStyle3>(int top, int left, int height, int width, std::string_view title);
template void DrawWindow<WindowStyle::kStyle4>(int top, int left, int height, int width, std::string_view title);

ColorId GetColor(char c) {
    static const std::unordered_map<char, ColorId> color_map{
        {'I', ColorId::kBrightCyan},   {'J', ColorId::kBrightBlue}, {'L', ColorId::kOrange},
        {'O', ColorId::kBrightYellow}, {'S', ColorId::kGreen},      {'T', ColorId::kMagenta},
        {'Z', ColorId::kBrightRed},
    };
    auto iter = color_map.find(c);
    if (iter != color_map.end()) {
        return iter->second;
    }
    return ColorId::kBlack;
}

template <std::size_t M, std::size_t N>
void DrawTetromino(const v1::Tetromino<M, N>& tetromino, int top, int left) {
    const auto& terminal = Terminal::GetInstance();
    for (std::size_t i = 0; i < M; i++) {
        for (std::size_t j = 0; j < N; j++) {
            if (tetromino[i][j] > '0') {
                terminal.MoveTo(top + i, Block2Col(left + j))
                    .SetBackgroundColor(GetColor(tetromino[i][j]))
                    .Output("  ")
                    .Reset();
            }
        }
    }
}

template void DrawTetromino(const v1::Tetromino<3, 3>& tetromino, int top, int left);
template void DrawTetromino(const v1::Tetromino<5, 5>& tetromino, int top, int left);

constexpr bool GetBit(int t, int i, int j) { return static_cast<bool>((1 << (i * 4 + j)) & t); }

void DrawTetromino(const v2::Tetromino& tetromino, int top, int left, int index) {
    const auto& terminal = Terminal::GetInstance();
    int data = tetromino.data[index];

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (GetBit(data, i, j)) {
                terminal.MoveTo(top + i, Block2Col(left + j))
                    .SetBackgroundColor(tetromino.color)
                    .Output("  ")
                    .Reset();
            }
        }
    }
}

void DrawTetromino(const v3::TetrominoSet& tetromino_set, int top, int left, int index) {
    const auto& terminal = Terminal::GetInstance();

    terminal.SetBackgroundColor(tetromino_set.color);

    // (dx, dy) -> (row, col)
    // row = row - dy;
    // col = col + dx;
    for (const auto& point : tetromino_set.data[index]) {
        terminal.MoveTo(top - point.y, Block2Col(left + point.x)).Output("  ");
    }
    terminal.Reset();
}

void DrawFrame(const Matrix& frame, int top, int left, bool clear_buffer) {
    /// @brief 初始化一个值均为 std::numeric_limits<int>::max() 的数组作为上一帧的缓存
    static Matrix prev_frame(kPlayFieldRow - 2, std::vector<int>(kPlayFieldCol, std::numeric_limits<int>::max()));
    if (clear_buffer) {
        prev_frame = std::vector<std::vector<int>>(
            kPlayFieldRow - 2, std::vector<int>(kPlayFieldCol, std::numeric_limits<int>::max()));
    }

    const auto& terminal = Terminal::GetInstance();

    constexpr std::size_t kRenderRow = kPlayFieldRow - 2;
    for (std::size_t y = 0; y < kRenderRow; y++) {
        for (std::size_t x = 0; x < kPlayFieldCol; x++) {
            if (prev_frame[y][x] == frame[y][x]) [[likely]] {
                continue;
            }
            prev_frame[y][x] = frame[y][x];

            int row = top + kRenderRow - y - 1;
            int col = left + x;
            terminal.MoveTo(row, Block2Col(col)).Reset();
            if (frame[y][x] > 0) {
                terminal.SetBackgroundColor(static_cast<ColorId>(frame[y][x])).Output("  ");
            } else if (frame[y][x] < 0) {
                terminal.SetColor(static_cast<ColorId>(-frame[y][x])).Output("**");
            } else {
                terminal.Output("\u30FB");
            }
        }
    }
    terminal.Reset();
}

void DrawPreview(const Matrix& preview, int top, int left) {
    static Matrix prev_preview(kPreviewRow, std::vector<int>(kPreviewCol, std::numeric_limits<int>::max()));

    const auto& terminal = Terminal::GetInstance();

    for (std::size_t y = 0; y < kPreviewRow; y++) {
        for (std::size_t x = 0; x < kPreviewCol; x++) {
            if (prev_preview[y][x] == preview[y][x]) [[likely]] {
                continue;
            }
            prev_preview[y][x] = preview[y][x];

            int row = top + kPreviewRow - y - 1;
            int col = left + x;
            terminal.MoveTo(row, Block2Col(col)).Reset();
            if (preview[y][x] > 0) {
                terminal.SetBackgroundColor(static_cast<ColorId>(preview[y][x])).Output("  ");
            } else {
                terminal.Output("  ");
            }
        }
    }
    terminal.Reset();
}

void DrawHold(const Matrix& hold, int top, int left) {
    static Matrix prev(kHoldRow, std::vector<int>(kHoldCol, std::numeric_limits<int>::max()));

    const auto& terminal = Terminal::GetInstance();

    for (std::size_t y = 0; y < kHoldRow; y++) {
        for (std::size_t x = 0; x < kHoldCol; x++) {
            if (prev[y][x] == hold[y][x]) [[likely]] {
                continue;
            }
            prev[y][x] = hold[y][x];

            int row = top + kHoldRow - y - 1;
            int col = left + x;
            terminal.MoveTo(row, Block2Col(col)).Reset();
            if (hold[y][x] > 0) {
                terminal.SetBackgroundColor(static_cast<ColorId>(hold[y][x])).Output("  ");
            } else {
                terminal.Output("  ");
            }
        }
    }
    terminal.Reset();
}

}  // namespace tetris
}  // namespace pyc
