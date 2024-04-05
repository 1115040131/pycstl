#include "tetris/engine.h"

#include <thread>

#include "tetris/terminal.h"

namespace pyc {
namespace tetris {

using namespace std::literals::chrono_literals;

void Engine::Init() {
    Terminal::HideCursor();
    prev_time_ = std::chrono::steady_clock::now();
}

void Engine::Loop() {
    int i = 1;
    while (true) {
        auto end_time = std::chrono::steady_clock::now();
        auto delta = end_time - prev_time_;
        prev_time_ = end_time;
        // lag_ += delta;

        Terminal::Clear();

        UpdateFps(delta);

        Terminal::GetInstance()
            .move_to(i++ % 20, 10)
            .set_background_color(ColorId::kBrightWhite)
            .output("   ")
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

    Terminal::GetInstance().move_to(1, 1).output(fmt::format("FPS: {}", fps_));
}

}  // namespace tetris
}  // namespace pyc
