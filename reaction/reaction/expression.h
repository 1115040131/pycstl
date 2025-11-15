#pragma once

#include "reaction/resource.h"

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

template <typename Fun, typename... Args>
class Expression : public Resource<ReturnType<Fun, Args...>> {
public:
    using ExprType = CalcExpr;
    using ValueType = ReturnType<Fun, Args...>;

    template <typename F, typename... A>
    void setSource(F&& fun, A&&... args) {
        static_assert(std::convertible_to<ReturnType<std::decay_t<F>, std::decay_t<A>...>, ValueType>,
                      "ReturnType<std::decay_t<F>, std::decay_t<A>...> should convert to ValueType");

#ifdef USE_FUNCTION
        this->updateObservers([this]() { this->valueChanged(); }, args.getPtr()...);
#else
        this->updateObservers(args.getPtr()...);
#endif
        setFunctor(createFun(std::forward<F>(fun), std::forward<A>(args)...));
        evaluate();
    }

    void addObCb(NodePtr node) {
#ifdef USE_FUNCTION
        this->updateObservers([this]() { this->valueChanged(); }, node);
#else
        this->updateObservers(node);
#endif
    }

private:
#ifdef USE_FUNCTION
    void valueChanged() {
        evaluate();
        this->notify();
    }
#else
    void valueChanged() override {
        evaluate();
        this->notify();
    }
#endif

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

    void evaluate() {
        if constexpr (VoidType<ValueType>) {
            std::invoke(fun_);
        } else {
            this->updateValue(std::invoke(fun_));
        }
    }

    void setFunctor(const std::function<ValueType()>& fun) { fun_ = fun; }

private:
    std::function<ValueType()> fun_;
};

template <NonInvocableType T>
class Expression<T> : public Resource<T> {
public:
    using ExprType = VarExpr;
    using ValueType = T;

    using Resource<T>::Resource;
};

template <typename Op, typename L, typename R>
class Expression<BinaryOpExpr<Op, L, R>>
    : public Expression<std::function<std::common_type_t<typename L::ValueType, typename R::ValueType>()>> {
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