#pragma once

#include <iostream>
#include <optional>
#include <vector>

namespace pyc {

struct Inputer {
    virtual std::optional<int> fetch() = 0;
};

struct CinInputer : Inputer {
    std::optional<int> fetch() override {
        int val;
        std::cin >> val;
        return val;
    }
};

struct VectorInputer : Inputer {
    std::vector<int> v;
    size_t index = 0;

    VectorInputer(std::vector<int> v) : v(v) {}

    std::optional<int> fetch() override {
        if (index >= v.size()) {
            return std::nullopt;
        }
        return v[index++];
    }
};

}  // namespace pyc
