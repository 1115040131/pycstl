#pragma once

#include <fmt/core.h>

#include "common/singleton.h"
#include "tetris/ansi.h"

namespace pyc {
namespace tetris {

class Terminal : public Singleton<Terminal> {
    friend class Singleton<Terminal>;

public:
    static void Flush() { fmt::println(""); }

    template <typename... Args>
    static void Output(fmt::format_string<Args...> fmt, Args&&... args) {
        fmt::print(fmt, std::forward<Args>(args)...);
    }

    static void MoveTo(int line, int column) {
        { fmt::print("{}", ::pyc::tetris::MoveTo(line, column)); }
    }

    static void Clear() { fmt::print("{}", kClear); }

    static void Reset() { fmt::print("{}", kReset); }

    static void SetColor(ColorId color_id) { fmt::print("{}", ::pyc::tetris::SetColor(color_id)); }

    static void SetBackgroundColor(ColorId color_id) {
        fmt::print("{}", ::pyc::tetris::SetBackgroundColor(color_id));
    }

    static void HideCursor() { fmt::print("{}", kCursorInvisible); }

    static void ShowCursor() { fmt::print("{}", kCursorVisible); }

    void flush() const { Flush(); }

    template <typename... Args>
    const Terminal& output(fmt::format_string<Args...> fmt, Args&&... args) const {
        Output(fmt, std::forward<Args>(args)...);
        return *this;
    }

    const Terminal& move_to(int line, int column) const {
        MoveTo(line, column);
        return *this;
    }

    const Terminal& clear() const {
        Clear();
        return *this;
    }

    const Terminal& reset() const {
        Reset();
        return *this;
    }

    const Terminal& set_color(ColorId color_id) const {
        SetColor(color_id);
        return *this;
    }

    const Terminal& set_background_color(ColorId color_id) const {
        SetBackgroundColor(color_id);
        return *this;
    }

    const Terminal& hide_cursor() const {
        HideCursor();
        return *this;
    }

    const Terminal& show_cursor() const {
        ShowCursor();
        return *this;
    }
};

}  // namespace tetris
}  // namespace pyc
