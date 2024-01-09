#pragma once

#include <memory>
#include <type_traits>

namespace pycstl {

template <typename FnSig>
struct Function {
    // 对于只有一个模板参数的 Function, 不符合函数签名的定义,
    // 会特化成该版本, static_assert 为 false, 编译不通过
    static_assert(!std::is_same_v<FnSig, FnSig>, "not a valid function signature");
};

template <typename Ret, typename... Args>
class Function<Ret(Args...)> {
public:
    // 通过虚函数擦除类型, base_ 在调用时都只需要调用 call 函数
    struct FuncBase {
        virtual Ret call(Args&&... args) = 0;
        virtual ~FuncBase() = default;
    };

    template <typename F>
    struct FuncImpl : FuncBase {
        F f;

        FuncImpl(F _f) : f(std::move(_f)) {}

        virtual Ret call(Args&&... args) override {
            return std::invoke(f, std::forward<Args>(args)...);
        }
    };

    Function() = default;

    template <typename F>
        requires(std::is_invocable_r_v<Ret, F, Args...> &&
                 !std::is_same_v<std::decay_t<F>, Function>)
    Function(F f) : base_(std::make_shared<FuncImpl<F>>(std::move(f))) {}

    Ret operator()(Args... args) const {
        if (!base_) [[unlikely]] {
            throw std::runtime_error("function uninitialized");
        }
        return base_->call(std::forward<Args>(args)...);
    }

private:
    std::shared_ptr<FuncBase> base_;
};

}  // namespace pycstl
