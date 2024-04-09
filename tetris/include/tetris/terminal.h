#pragma once

#include <iostream>

#include "common/singleton.h"
#include "tetris/ansi.h"

namespace pyc {
namespace tetris {

class Terminal : public Singleton<Terminal> {
    friend class Singleton<Terminal>;

public:
    void Flush(std::ostream& os = std::cout) const { os << std::flush; }

    const Terminal& Output(std::string_view msg, std::ostream& os = std::cout) const {
        os << msg;
        return *this;
    }

    const Terminal& MoveTo(int line, int column, std::ostream& os = std::cout) const {
        os << ::pyc::tetris::MoveTo(line, column);
        return *this;
    }

    const Terminal& Clear(std::ostream& os = std::cout) const {
        os << kClear;
        return *this;
    }

    const Terminal& Reset(std::ostream& os = std::cout) const {
        os << kReset;
        return *this;
    }

    const Terminal& SetColor(ColorId color_id, std::ostream& os = std::cout) const {
        os << ::pyc::tetris::SetColor(color_id);
        return *this;
    }

    const Terminal& SetBackgroundColor(ColorId color_id, std::ostream& os = std::cout) const {
        os << ::pyc::tetris::SetBackgroundColor(color_id);
        return *this;
    }

    const Terminal& HideCursor(std::ostream& os = std::cout) const {
        os << kCursorInvisible;
        return *this;
    }

    const Terminal& ShowCursor(std::ostream& os = std::cout) const {
        os << kCursorVisible;
        return *this;
    }
};

}  // namespace tetris
}  // namespace pyc
