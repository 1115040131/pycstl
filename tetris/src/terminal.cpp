#include "tetris/terminal.h"

#include <fmt/core.h>

namespace pyc {
namespace tetris {

void Terminal::Flush() { fmt::println(""); }

void Terminal::Output(std::string_view msg) { fmt::print("{}", msg); }

void Terminal::MoveTo(int line, int column) { fmt::print("{}", ::pyc::tetris::MoveTo(line, column)); }

void Terminal::Clear() { fmt::print("{}", kClear); }

void Terminal::Reset() { fmt::print("{}", kReset); }

void Terminal::SetColor(ColorId color_id) { fmt::print("{}", ::pyc::tetris::SetColor(color_id)); }

void Terminal::SetBackgroundColor(ColorId color_id) {
    fmt::print("{}", ::pyc::tetris::SetBackgroundColor(color_id));
}

void Terminal::HideCursor() { fmt::print("{}", kCursorInvisible); }

void Terminal::ShowCursor() { fmt::print("{}", kCursorVisible); }

}  // namespace tetris
}  // namespace pyc
