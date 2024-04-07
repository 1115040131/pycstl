#include "tetris/engine.h"

#include <thread>

#include "tetris/draw.h"
#include "tetris/terminal.h"

namespace pyc {
namespace tetris {

using namespace std::literals::chrono_literals;

void Engine::Init() {
    Terminal::HideCursor();
    Terminal::Clear();

    prev_time_ = std::chrono::steady_clock::now();

    Window<WindowStyle::kStyle1>(1, 1, 9, 6, "Hold");
    Window<WindowStyle::kStyle2>(1, 10, 12, 22, "Tetriz");
    Window<WindowStyle::kStyle3>(7, 1, 9, 16, "Status");
    Window<WindowStyle::kStyle4>(19, 22, 8, 4, "Info");
    Window(1, 22, 8, 18, "Next");
}

void Engine::Loop() {
    int i = 4;
    while (true) {
        auto end_time = std::chrono::steady_clock::now();
        auto delta = end_time - prev_time_;
        prev_time_ = end_time;
        // lag_ += delta;

        UpdateFps(delta);

        Terminal::GetInstance()
            .move_to(i % 20, 10)
            .set_background_color(ColorId::kBrightWhite)
            .output("  ")
            .reset()
            .flush();
        std::this_thread::sleep_for(100ms);
    }
}

void Engine::Exit() { Terminal::GetInstance().show_cursor().reset().flush(); }

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
