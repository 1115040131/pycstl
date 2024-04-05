#include "tetris/draw.h"

#include <fmt/core.h>

#include "tetris/terminal.h"

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

inline int Block2Col(int block_idx) { return 2 * block_idx - 1; }

void Window(int top, int left, int height, int width, std::string_view title) {
    if (width < 2 || height < 2) {
        fmt::println("Draw window error,width: {}, left: {}", width, height);
        return;
    }

    const auto& termianl = Terminal::GetInstance();

    // 第一行
    termianl.move_to(top, Block2Col(left)).output(fmt::format(" ┌{:─^{}}┐ ", title, 2 * width - 4));

    for (int i = 1; i < height - 1; i++) {
        termianl.move_to(top + i, Block2Col(left)).output(fmt::format(" │{:<{}}│ ", "", 2 * width - 4));
    }

    // 最后一行
    termianl.move_to(top + height - 1, Block2Col(left)).output(fmt::format(" └{:─<{}}┘ ", "", 2 * width - 4));
}

}  // namespace tetris
}  // namespace pyc
