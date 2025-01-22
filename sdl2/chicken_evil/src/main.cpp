#include "chicken_evil/engine.h"

#define ASSET_PATH "sdl2/demo/assets/"
#define ASSET(filename) ("sdl2/demo/assets/" filename)

int main() {
    pyc::sdl2::Engine engine;
    engine.init();
    engine.mainloop();
    engine.deinit();

    return 0;
}