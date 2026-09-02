#include "tetris/window.h"

#include "tetris/draw.h"
#include "tetris/game.h"
#include "tetris/terminal.h"
#include "tetris/utils.h"

namespace pyc {
namespace tetris {

void ShowWindows() {
    Terminal::GetInstance().HideCursor().Clear();
    DrawWindow<WindowStyle::kStyle1>(1, 1, 9, 6, "Hold");
    DrawWindow<WindowStyle::kStyle2>(1, 10, 12, 22, "Tetriz");
    DrawWindow<WindowStyle::kStyle3>(7, 1, 9, 16, "Status");
    DrawWindow<WindowStyle::kStyle4>(19, 22, 8, 4, "Info");
    DrawWindow(1, 22, 8, 18, "Next");
}

void ShowInfo() { Terminal::GetInstance().MoveTo(21, Block2Col(24)).Output("Help [h]"); }

void ShowStatus() {
    const auto& terminal = Terminal::GetInstance();
    const auto& game = Game::GetInstance();

    terminal.MoveTo(10, 4)
        .Output(fmt::format("FPS: {:<5}", game.FPS()))
        .MoveTo(12, 4)
        .Output(fmt::format("Level: {:<5}", game.Level()))
        .MoveTo(13, 4)
        .Output(fmt::format("Score: {:<5}", game.Score()))
        .MoveTo(14, 4)
        .Output(fmt::format("Lines: {:<5}", game.Lines()));

    // 失败时绘制一次
    static bool prev_ending = game.Ending();
    if (!prev_ending && game.Ending()) {
        DrawWindow<WindowStyle::kStyle4>(9, 12, 8, 3, "");
        terminal.MoveTo(10, 26).SetColor(ColorId::kBrightRed).Output("Game Over!").Reset();
    }
    prev_ending = game.Ending();

    static bool prev_helping = game.Helping();
    if (!prev_helping && game.Helping()) {
        static constexpr std::array<std::string_view, 9> contents = {
            // clang-format off
            "Help Info:      ",
            "                ",
            "Rotate   [r]    ",
            "Left     [a]    ",
            "Right    [d]    ",
            "Down     [s]    ",
            "Drop   [space]  ",
            "Hold     [c]    ",
            "Reset    [r]    ",
            // clang-format on
        };
        int row = 5;
        for (auto content : contents) {
            terminal.MoveTo(row++, 23).Output(content);
        }
    }
}

void ShowGame() {
    const auto& game = Game::GetInstance();

    // 重置时或关闭help时重置缓存
    static bool prev_ending = game.Ending();
    static bool prev_helping = game.Helping();

    bool clear_buffer = (prev_ending && !game.Ending()) || (prev_helping && !game.Helping());
    prev_ending = game.Ending();
    prev_helping = game.Helping();

    DrawFrame(game.GetFrame(), 2, 11, clear_buffer);
    DrawPreview(game.GetPreviewField(), 2, 23);
    DrawHold(game.GetHoldField(), 2, 2);

    Terminal::GetInstance().Flush();
}

void ShowExit() {
    Terminal::GetInstance()
        .ShowCursor()
        .Reset()
        .Clear()
        .MoveTo(1, 1)
        .SetColor(ColorId::kBrightRed)
        .Output("Thank you for playing Tetris!\n")
        .Flush();
}

}  // namespace tetris
}  // namespace pyc
