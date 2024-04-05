
#include <chrono>
#include <thread>

#include "tetris/terminal.h"

using namespace std::literals::chrono_literals;

using namespace pyc::tetris;

int main() {
    Terminal::HideCursor();
    int i = 1;
    while (true) {
        Terminal::GetInstance()
            .clear()
            .move_to(i++, 10)
            .set_background_color(ColorId::kBrightWhite)
            .output("   ")
            .reset()
            .flush();
        std::this_thread::sleep_for(300ms);
    }

    return 0;
}