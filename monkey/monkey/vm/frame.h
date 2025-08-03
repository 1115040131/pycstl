#pragma once

#include "monkey/code/code.h"
#include "monkey/object/object.h"

namespace pyc {
namespace monkey {

struct Frame {
    std::shared_ptr<CompiledFunction> function;
    size_t ip{};
    size_t bp{};

    static std::shared_ptr<Frame> New(std::shared_ptr<CompiledFunction> function_, size_t bp_) {
        return std::make_shared<Frame>(std::move(function_), 0, bp_);
    }

    const Instructions& instructions() const { return function->instructions(); }
};

}  // namespace monkey
}  // namespace pyc