#include "tetris/engine.h"

#include <thread>

#include "tetris/control.h"
#include "tetris/draw.h"
#include "tetris/game.h"
#include "tetris/terminal.h"

namespace pyc {
namespace tetris {

using namespace std::literals::chrono_literals;

void Engine::Init() {
    // 绘制窗口
    Terminal::GetInstance().HideCursor().Clear();
    DrawWindow<WindowStyle::kStyle1>(1, 1, 9, 6, "Hold");
    DrawWindow<WindowStyle::kStyle2>(1, 10, 12, 22, "Tetriz");
    DrawWindow<WindowStyle::kStyle3>(7, 1, 9, 16, "Status");
    DrawWindow<WindowStyle::kStyle4>(19, 22, 8, 4, "Info");
    DrawWindow(1, 22, 8, 18, "Next");

    Game::GetInstance().Init();
    Control::StartListener();  // 开启键盘监听
}

void Engine::Loop() {
    auto& game = Game::GetInstance();

    auto prev_time = std::chrono::steady_clock::now();
    while (Game::GetInstance().Running()) {
        auto end_time = std::chrono::steady_clock::now();
        auto delta = end_time - prev_time;
        prev_time = end_time;

        game.Process(delta);

        Update(delta);

        std::this_thread::sleep_for(10ms);
    }
}

void Engine::Exit() {
    Terminal::GetInstance()
        .ShowCursor()
        .Reset()
        .Clear()
        .MoveTo(1, 1)
        .SetColor(ColorId::kBrightRed)
        .Output("Bye!\n")
        .Flush();
}

void Engine::Update(std::chrono::nanoseconds delta) {
    static std::chrono::nanoseconds total{};
    static std::size_t frame_count{};
    total += delta;
    frame_count++;

    const auto& terminal = Terminal::GetInstance();
    const auto& game = Game::GetInstance();

    if (total >= 1s) {
        terminal.MoveTo(10, 4).Output(fmt::format(
            "FPS: {:<3}", frame_count / std::chrono::duration_cast<std::chrono::seconds>(total).count()));

        total = std::chrono::nanoseconds{};
        frame_count = 0;
    }

    terminal.MoveTo(12, 4)
        .Output(fmt::format("Level: {}", game.Level()))
        .MoveTo(13, 4)
        .Output(fmt::format("Score: {}", game.Score()))
        .MoveTo(14, 4)
        .Output(fmt::format("Lines: {}", game.Lines()));

    static bool draw_end = false;
    if (game.Ending() && !draw_end) {
        draw_end = true;
        DrawWindow<WindowStyle::kStyle4>(9, 12, 8, 3, "");
        terminal.MoveTo(10, 26).SetColor(ColorId::kBrightRed).Output("Game Over!").Reset();
    }
}

}  // namespace tetris
}  // namespace pyc
