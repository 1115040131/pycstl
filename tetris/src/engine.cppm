module;

export module tetris.engine;

import common.singleton;

export namespace pyc::tetris {

class Engine : public Singleton<Engine> {
    friend class Singleton<Engine>;

public:
    void Init();

    void Loop();

    void Exit();

private:
    Engine() = default;
};

}  // namespace pyc::tetris
