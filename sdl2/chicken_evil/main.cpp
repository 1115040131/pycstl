import sdl2.chicken_evil.engine;

int main() {
    pyc::sdl2::Engine engine;
    engine.init();
    engine.mainloop();
    engine.deinit();

    return 0;
}
