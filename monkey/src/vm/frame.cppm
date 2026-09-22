module;

#include <cstddef>
#include <memory>
#include <utility>

export module monkey.vm.frame;

export import monkey.code;
export import monkey.object;

export namespace pyc::monkey {

struct Frame {
    std::shared_ptr<Closure> closure;
    size_t ip{};
    size_t bp{};

    static std::shared_ptr<Frame> New(std::shared_ptr<Closure> closure_, size_t bp_) {
        return std::make_shared<Frame>(std::move(closure_), 0, bp_);
    }

    const Instructions& instructions() const { return closure->compiledFunction()->instructions(); }
};

}  // namespace pyc::monkey
