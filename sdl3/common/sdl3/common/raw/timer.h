#pragma once

#include "sdl3/common/core/object.h"

namespace pyc {
namespace sdl3 {

class Timer : public Object {
public:
    static Timer* CreateAndSet(Object* parent, std::chrono::duration<float> interval = 3s);

    virtual void update(std::chrono::duration<float> delta) override;

    void start() { is_active_ = true; }
    void stop() { is_active_ = false; }

    bool isTimeOut();

    float getProgress() const {
        return timer_.count() / interval_.count();
    }

private:
    std::chrono::duration<float> timer_{};
    std::chrono::duration<float> interval_{};
    bool time_out_{};
};

}  // namespace sdl3
}  // namespace pyc