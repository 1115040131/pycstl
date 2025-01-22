#pragma once

#include <chrono>
#include <functional>

namespace pyc {
namespace sdl2 {

class Timer {
public:
    Timer() = default;
    ~Timer() = default;

    void restart() {
        pass_time_ = std::chrono::duration<double>::zero();
        shotted_ = false;
    }

    void set_wait_time(std::chrono::duration<double> wait_time) { wait_time_ = wait_time; }

    void set_one_shot(bool one_shot) { one_shot_ = one_shot; }

    void set_on_timeout(std::function<void()> on_timeout) { on_timeout_ = on_timeout; }

    void pause() { paused_ = true; }

    void resume() { paused_ = false; }

    void on_update(std::chrono::duration<double> delta);

private:
    bool shotted_{false};                        // 是否触发过
    std::chrono::duration<double> pass_time_{};  // 经过时间

    std::chrono::duration<double> wait_time_;    // 等待时间
    bool one_shot_{false};                       // 是否触发一次
    bool paused_{false};                         // 是否暂停
    std::function<void()> on_timeout_{nullptr};  // 超时回调
};

}  // namespace sdl2
}  // namespace pyc