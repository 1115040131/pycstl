#pragma once

#include <chrono>

#include "common/singleton.h"

namespace pyc {
namespace tetris {

class Engine : public Singleton<Engine> {
    friend class Singleton<Engine>;

public:
    void Init();

    void Loop();

    void Exit();

private:
    void UpdateFps(std::chrono::nanoseconds delta);
};

}  // namespace tetris
}  // namespace pyc
