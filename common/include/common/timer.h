#pragma once

#include <chrono>
#include <source_location>

namespace pyc {

class Timer {
public:
    explicit Timer(const std::source_location& location = std::source_location::current())
        : location_(location), start_(std::chrono::high_resolution_clock::now()) {}

    ~Timer();

private:
    std::source_location location_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

}  // namespace pyc