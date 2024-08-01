#include "common/timer.h"

#include <fmt/chrono.h>

#include "common/utils.h"

namespace pyc {

Timer::~Timer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
    fmt::println("[{}]: Elapsed time: {}.{:06} seconds", GetSimpleName(location_.function_name()),
                 duration.count() / 1000000, duration.count() % 1000000);
}

}  // namespace pyc
