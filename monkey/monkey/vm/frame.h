#pragma once

#include "monkey/code/code.h"
#include "monkey/object/object.h"

namespace pyc {
namespace monkey {

struct Frame {
    std::shared_ptr<Closure> closure;
    size_t ip{};
    size_t bp{};

    static std::shared_ptr<Frame> New(std::shared_ptr<Closure> closure_, size_t bp_) {
        return std::make_shared<Frame>(std::move(closure_), 0, bp_);
    }

    const Instructions& instructions() const { return closure->compiledFunction()->instructions(); }
};

}  // namespace monkey
}  // namespace pyc