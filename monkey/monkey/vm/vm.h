#pragma once

#include "monkey/compiler/compiler.h"
#include "monkey/vm/frame.h"

namespace pyc {
namespace monkey {

class VM {
public:
    static constexpr size_t kFrameSize = 1024;
    static constexpr size_t kStackSize = 2048;
    static constexpr size_t kGlobalSize = 65536;

    static std::shared_ptr<VM> New(std::shared_ptr<Compiler> compiler);

    static std::shared_ptr<VM> NewWithState(std::shared_ptr<Compiler> compiler,
                                            const std::vector<std::shared_ptr<Object>>& globals) {
        auto vm = New(compiler);
        vm->globals_ = globals;
        return vm;
    }

    VM(const std::vector<std::shared_ptr<Object>>& constants, const std::vector<std::shared_ptr<Frame>>& frames)
        : constants_(constants), globals_(kGlobalSize), stack_(kStackSize), frames_(frames) {}

    const std::vector<std::shared_ptr<Object>>& globals() const { return globals_; }

public:
    std::shared_ptr<Object> top() const;

    std::shared_ptr<Object> lastPoppedElement() const;

    std::shared_ptr<Object> push(std::shared_ptr<Object> object);

    std::shared_ptr<Object> pop();

    std::shared_ptr<Object> run();

private:
    std::shared_ptr<Object> excuteBinaryOperation(OpcodeType op);

    std::shared_ptr<Object> excuteBinaryIntegerOperation(OpcodeType op, std::shared_ptr<Integer> left,
                                                         std::shared_ptr<Integer> right);

    std::shared_ptr<Object> excuteBinaryStringOperation(OpcodeType op, std::shared_ptr<String> left,
                                                        std::shared_ptr<String> right);

    std::shared_ptr<Object> excuteComparison(OpcodeType op);

    std::shared_ptr<Object> excuteIntegerComparison(OpcodeType op, std::shared_ptr<Integer> left,
                                                    std::shared_ptr<Integer> right);

    std::shared_ptr<Object> excuteBangOperation();

    std::shared_ptr<Object> excuteMinusOperation();

    std::shared_ptr<Object> executeIndexExpression();

    std::shared_ptr<Object> buildArray(size_t size);

    std::shared_ptr<Object> buildHash(size_t size);

    std::shared_ptr<Object> executeCall(size_t num_args);

    std::shared_ptr<Object> callClosure(std::shared_ptr<Closure> closure, size_t num_args);

    std::shared_ptr<Object> callBuiltin(std::shared_ptr<Builtin> builtin, size_t num_args);

    std::shared_ptr<Object> pushClosure(size_t const_index, size_t free_num);

    std::shared_ptr<Frame> currentFrame() { return frames_[frame_index_]; }

    void pushFrame(std::shared_ptr<Frame> frame);
    std::shared_ptr<Frame> popFrame();

private:
    std::vector<std::shared_ptr<Object>> constants_;  // 常量
    std::vector<std::shared_ptr<Object>> globals_;    // 全局变量

    std::vector<std::shared_ptr<Object>> stack_;
    size_t sp_{};

    std::vector<std::shared_ptr<Frame>> frames_;
    size_t frame_index_{};
};

}  // namespace monkey
}  // namespace pyc