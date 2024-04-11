#include "tetris/engine.h"

#include <thread>

#include "tetris/control.h"
#include "tetris/game.h"
#include "tetris/window.h"

namespace pyc {
namespace tetris {

using namespace std::literals::chrono_literals;

void Engine::Init() {
    ShowWindows();
    ShowInfo();
    Game::GetInstance().Init();
    Control::StartListener();  // 开启键盘监听
}

void Engine::Loop() {
    auto prev_time = std::chrono::steady_clock::now();
    while (Game::GetInstance().Running()) {
        auto end_time = std::chrono::steady_clock::now();
        auto delta = end_time - prev_time;
        prev_time = end_time;

        Game::GetInstance().Process(delta);
        ShowStatus();
        ShowGame();

        std::this_thread::sleep_for(10ms);
    }
}

void Engine::Exit() { ShowExit(); }

}  // namespace tetris
}  // namespace pyc
