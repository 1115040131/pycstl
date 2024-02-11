#pragma once

#include <chrono>
#include <source_location>

#include <fmt/chrono.h>
#include <fmt/core.h>

class Timer {
public:
    explicit Timer(const std::source_location& location = std::source_location::current())
        : location_(location), start_(std::chrono::high_resolution_clock::now()) {}

    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start_);
        fmt::println("[{}]: Elapsed time: {}.{:06} seconds", location_.function_name(), duration.count() / 1000000,
                     duration.count() % 1000000);
    }

private:
    std::source_location location_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};