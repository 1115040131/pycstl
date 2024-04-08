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
    Terminal::HideCursor();
    Control::StartListener();  // 开启键盘监听
    Game::GetInstance().Init();

    prev_time_ = std::chrono::steady_clock::now();
}

void Engine::Loop() {
    const auto& game = Game::GetInstance();

    while (Game::GetInstance().Running()) {
        auto end_time = std::chrono::steady_clock::now();
        auto delta = end_time - prev_time_;
        prev_time_ = end_time;
        // lag_ += delta;

        // 重绘窗口
        Terminal::Clear();
        UpdateFps(delta);

        DrawWindow<WindowStyle::kStyle1>(1, 1, 9, 6, "Hold");
        DrawWindow<WindowStyle::kStyle2>(1, 10, 12, 22, "Tetriz");
        DrawWindow<WindowStyle::kStyle3>(7, 1, 9, 16, "Status");
        DrawWindow<WindowStyle::kStyle4>(19, 22, 8, 4, "Info");
        DrawWindow(1, 22, 8, 18, "Next");

        DrawTetromino(game.Curr(), game.Row(), game.Col(), game.Index());

        Terminal::Flush();
        std::this_thread::sleep_for(50ms);
    }
}

void Engine::Exit() {
    Terminal::GetInstance()
        .show_cursor()
        .reset()
        .clear()
        .move_to(1, 1)
        .set_color(ColorId::kBrightRed)
        .output("Bye!")
        .flush();
}

void Engine::UpdateFps(std::chrono::nanoseconds delta) {
    static std::chrono::nanoseconds total{};
    static std::size_t frame_count{};
    total += delta;
    frame_count++;
    if (total >= 1s) {
        fps_ = frame_count / std::chrono::duration_cast<std::chrono::seconds>(total).count();
        total = std::chrono::nanoseconds{};
        frame_count = 0;
    }

    Terminal::GetInstance().move_to(10, 4).output("FPS: {}", fps_);
}

}  // namespace tetris
}  // namespace pyc
