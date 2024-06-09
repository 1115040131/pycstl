#pragma once

#include "design_pattern/inputer.h"
#include "design_pattern/reducer.h"

namespace pyc {

inline int reduce(std::vector<int> v, Reducer* reducer) {
    int res = reducer->init();
    for (size_t i = 0; i < v.size(); i++) {
        res = reducer->add(res, v[i]);
    }
    return res;
}

inline int reduce(Inputer* inputer, Reducer* reducer) {
    int res = reducer->init();
    while (auto val = inputer->fetch()) {
        res = reducer->add(res, *val);
    }
    return res;
}

}  // namespace pyc
