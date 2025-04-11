#pragma once

#include <atomic>

namespace pyc {
namespace sdl3 {

template <typename T>
int GetCount() {
    static std::atomic<int> count = 0;
    return count++;
}

}  // namespace sdl3
}  // namespace pyc