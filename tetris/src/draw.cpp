#include "tetris/draw.h"

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
void Window(int top, int left, int width, int height, std::string_view title) {
    if (width < 2 || height < 2) {
        fmt::println("Draw window error,width: {}, left: {}", width, height);
        return;
    }

    const auto& terminal = Terminal::GetInstance();

    int padding_width = 2 * width - 4;

    if constexpr (style == WindowStyle::kStyle1) {
        terminal.move_to(top, Block2Col(left)).output(" ┌{:─^{}}┐ ", title, padding_width);
        for (int i = 1; i < height - 1; i++) {
            terminal.move_to(top + i, Block2Col(left)).output(" │{:<{}}│ ", "", padding_width);
        }
        terminal.move_to(top + height - 1, Block2Col(left)).output(" └{:─<{}}┘ ", "", padding_width);
    } else if constexpr (style == WindowStyle::kStyle2) {
        terminal.move_to(top, Block2Col(left)).output(" ╔{:═^{}}╗ ", title, padding_width);
        for (int i = 1; i < height - 1; i++) {
            terminal.move_to(top + i, Block2Col(left)).output(" ║{:<{}}║ ", "", padding_width);
        }
        terminal.move_to(top + height - 1, Block2Col(left)).output(" ╚{:═<{}}╝ ", "", padding_width);
    } else if constexpr (style == WindowStyle::kStyle3) {
        terminal.move_to(top, Block2Col(left)).output(" ┏{:━^{}}┓ ", title, padding_width);
        for (int i = 1; i < height - 1; i++) {
            terminal.move_to(top + i, Block2Col(left)).output(" ┃{:<{}}┃ ", "", padding_width);
        }
        terminal.move_to(top + height - 1, Block2Col(left)).output(" ┗{:━<{}}┛ ", "", padding_width);
    } else if constexpr (style == WindowStyle::kStyle4) {
        terminal.move_to(top, Block2Col(left)).output(" ╭{:─^{}}╮ ", title, padding_width);
        for (int i = 1; i < height - 1; i++) {
            terminal.move_to(top + i, Block2Col(left)).output(" │{:<{}}│ ", "", padding_width);
        }
        terminal.move_to(top + height - 1, Block2Col(left)).output(" ╰{:─<{}}╯ ", "", padding_width);
    }
}

template void Window<WindowStyle::kStyle1>(int top, int left, int height, int width, std::string_view title);
template void Window<WindowStyle::kStyle2>(int top, int left, int height, int width, std::string_view title);
template void Window<WindowStyle::kStyle3>(int top, int left, int height, int width, std::string_view title);
template void Window<WindowStyle::kStyle4>(int top, int left, int height, int width, std::string_view title);

}  // namespace tetris
}  // namespace pyc
