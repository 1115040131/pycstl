#include "chicken_evil/engine.h"

int main() {
    pyc::sdl2::Engine engine;
    engine.init();
    engine.mainloop();
    engine.deinit();

    return 0;
}