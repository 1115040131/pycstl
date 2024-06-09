#pragma once

#include "design_pattern/inputer.h"
#include "design_pattern/reducer.h"

namespace pyc {

inline int reduce(std::vector<int> v, Reducer* reducer) {
    auto state = reducer->init();
    for (size_t i = 0; i < v.size(); i++) {
        state->add(v[i]);
    }
    return state->result();
}

inline int reduce(Inputer* inputer, Reducer* reducer) {
    auto state = reducer->init();
    while (auto val = inputer->fetch()) {
        state->add(val.value());
    }
    return state->result();
}

}  // namespace pyc
