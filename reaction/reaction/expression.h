#pragma once

#include "reaction/resource.h"
#include "reaction/trigger_mode.h"

namespace pyc::reaction {

struct VarExpr {};
struct CalcExpr {};

template <typename Op, typename L, typename R>
class BinaryOpExpr {
public:
    using ValueType = std::common_type_t<typename L::ValueType, typename R::ValueType>;

    template <typename Left, typename Right>
    BinaryOpExpr(Left&& left, Right&& right, Op op = Op{})
        : left_(std::forward<Left>(left)), right_(std::forward<Right>(right)), op_(op) {}

    auto operator()() const { return calculate(); }

private:
    auto calculate() const { return op_(left_(), right_()); }

private:
    L left_;
    R right_;
    [[no_unique_address]] Op op_;
};

struct AddOp {
    auto operator()(auto&& l, auto&& r) const { return l + r; }
};

struct SubOp {
    auto operator()(auto&& l, auto&& r) const { return l - r; }
};

struct MulOp {
    auto operator()(auto&& l, auto&& r) const { return l * r; }
};

struct DivOp {
    auto operator()(auto&& l, auto&& r) const { return l / r; }
};

template <typename Type>
struct ValueWrapper {
    using ValueType = Type;
    Type value;

    template <typename T>
    explicit ValueWrapper(T&& v) : value(std::forward<T>(v)) {}

    const Type& operator()() const { return value; }
};

template <typename Op, typename L, typename R>
auto make_binary_expr(L&& left, R&& right) {
    return BinaryOpExpr<Op, ExprWrapper<std::decay_t<L>>, ExprWrapper<std::decay_t<R>>>(std::forward<L>(left),
                                                                                        std::forward<R>(right));
}

template <typename L, typename R>
    requires HasCustomOp<L, R>
auto operator+(L&& left, R&& right) {
    return make_binary_expr<AddOp>(std::forward<L>(left), std::forward<R>(right));
}

template <typename L, typename R>
    requires HasCustomOp<L, R>
auto operator-(L&& left, R&& right) {
    return make_binary_expr<SubOp>(std::forward<L>(left), std::forward<R>(right));
}

template <typename L, typename R>
    requires HasCustomOp<L, R>
auto operator*(L&& left, R&& right) {
    return make_binary_expr<MulOp>(std::forward<L>(left), std::forward<R>(right));
}

template <typename L, typename R>
    requires HasCustomOp<L, R>
auto operator/(L&& left, R&& right) {
    return make_binary_expr<DivOp>(std::forward<L>(left), std::forward<R>(right));
}

template <IsTrigMode TrigMode, typename Fun, typename... Args>
class Expression : public Resource<ReturnType<TrigMode, Fun, Args...>>, public TrigMode {
public:
    using ExprType = CalcExpr;
    using ValueType = ReturnType<TrigMode, Fun, Args...>;

    template <typename F, typename... A>
    void setSource(F&& fun, A&&... args) {
        static_assert(std::convertible_to<ReturnType<TrigMode, std::decay_t<F>, std::decay_t<A>...>, ValueType>,
                      "ReturnType<TrigMode, std::decay_t<F>, std::decay_t<A>...> should convert to ValueType");

        this->updateObservers(args.getPtr()...);
        setFunctor(createFun(std::forward<F>(fun), std::forward<A>(args)...));

        if constexpr (!VoidType<ValueType>) {
            this->updateValue(evaluate());
        } else {
            evaluate();
        }
    }

    void addObCb(NodePtr node) { this->updateObservers(node); }

private:
    template <typename F, typename... A>
    auto createFun(F&& fun, A&&... args) {
        return [fun = std::forward<F>(fun), ... args = args.getPtr()]() {
            if constexpr (VoidType<ValueType>) {
                std::invoke(fun, args->get()...);
                return VoidWrapper{};
            } else {
                return std::invoke(fun, args->get()...);
            }
        };
    }

    void valueChanged(bool changed = true) override {
        if constexpr (std::is_same_v<ChangeTrig, TrigMode>) {
            this->setChanged(changed);
        }
        if (TrigMode::checkTrigger()) {
            if constexpr (!VoidType<ValueType>) {
                auto old_value = this->getValue();
                auto new_value = evaluate();
                this->updateValue(new_value);
                if constexpr (Cmparable<ValueType>) {
                    this->notify(old_value != new_value);
                } else {
                    this->notify(true);
                }
            } else {
                evaluate();
                this->notify(true);
            }
        }
    }

    auto evaluate() {
        if constexpr (VoidType<ValueType>) {
            std::invoke(fun_);
        } else {
            return std::invoke(fun_);
        }
    }

    void setFunctor(const std::function<ValueType()>& fun) { fun_ = fun; }

private:
    std::function<ValueType()> fun_;
};

template <IsTrigMode TrigMode, NonInvocableType T>
class Expression<TrigMode, T> : public Resource<T> {
public:
    using ExprType = VarExpr;
    using ValueType = T;

    using Resource<T>::Resource;
};

template <IsTrigMode TrigMode, typename Op, typename L, typename R>
class Expression<TrigMode, BinaryOpExpr<Op, L, R>>
    : public Expression<TrigMode,
                        std::function<std::common_type_t<typename L::ValueType, typename R::ValueType>()>> {
public:
    template <typename T>
    explicit Expression(T&& expr) : expr_(std::forward<T>(expr)) {}

protected:
    void setOpExpr() {
        this->setSource([this]() { return expr_(); });
    }

private:
    BinaryOpExpr<Op, L, R> expr_;
};

}  // namespace pyc::reaction