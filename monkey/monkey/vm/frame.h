#pragma once

#include "monkey/code/code.h"
#include "monkey/object/object.h"

namespace pyc {
namespace monkey {

struct Frame {
    std::shared_ptr<CompiledFunction> function;
    size_t ip{};

    static std::shared_ptr<Frame> New(std::shared_ptr<CompiledFunction> function_) {
        return std::make_shared<Frame>(std::move(function_));
    }

    Frame(std::shared_ptr<CompiledFunction> function_) : function(std::move(function_)) {}

    const Instructions& instructions() const { return function->instructions(); }
};

}  // namespace monkey
}  // namespace pyc