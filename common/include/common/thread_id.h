#pragma once

#include <sstream>
#include <string>
#include <thread>

namespace pyc {

inline const std::string& ThreadId() {
    thread_local std::string thread_id;
    if (thread_id.empty()) {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        thread_id = oss.str();
    }
    return thread_id;
}

// NOTE: Uses hash modulo, so collisions between threads are possible.
// This is intentional — the short ID is for human readability only, not uniqueness.
inline std::size_t ShortThreadId() {
    constexpr std::size_t kMaxId = 100000;
    thread_local std::size_t short_thread_id{kMaxId};
    if (short_thread_id == kMaxId) {
        short_thread_id = std::hash<std::thread::id>{}(std::this_thread::get_id()) % kMaxId;
    }
    return short_thread_id;
}

}  // namespace pyc
