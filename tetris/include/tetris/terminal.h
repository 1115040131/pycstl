#pragma once

#include <string_view>

#include "common/singleton.h"
#include "tetris/ansi.h"

namespace pyc {
namespace tetris {

class Terminal : public Singleton<Terminal> {
    friend class Singleton<Terminal>;

public:
    static void Flush();
    static void Output(std::string_view msg);
    static void MoveTo(int line, int column);
    static void Clear();
    static void Reset();
    static void SetColor(ColorId color_id);
    static void SetBackgroundColor(ColorId color_id);
    static void HideCursor();
    static void ShowCursor();

    void flush() const { Flush(); }
    const Terminal& output(std::string_view msg) const {
        Output(msg);
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
