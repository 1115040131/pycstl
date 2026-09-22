module;

#include <string>
#include <vector>

export module monkey.util;

export namespace pyc::monkey {

inline std::string Join(const std::vector<std::string>& nodes, std::string dim) {
    std::string connect;
    for (const auto& node : nodes) {
        connect += node + dim;
    }
    if (!connect.empty()) {
        connect.pop_back();
        connect.pop_back();
    }
    return connect;
}

}  // namespace pyc::monkey
