#include "tetris/engine.h"

int main() {
    auto& engine = pyc::tetris::Engine::GetInstance();

    engine.Init();

    engine.Loop();

    engine.Exit();

    return 0;
}