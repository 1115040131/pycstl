import tetris.engine;

int main() {
    auto& engine = pyc::tetris::Engine::GetInstance();

    engine.Init();

    engine.Loop();

    engine.Exit();

    return 0;
}