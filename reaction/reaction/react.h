#pragma once

#include <atomic>

#include "reaction/expression.h"
#include "reaction/utility.h"

namespace pyc::reaction {

inline thread_local std::function<void(NodePtr)> g_reg_fun;

struct RegGuard {
    RegGuard(const std::function<void(NodePtr)>& func) { g_reg_fun = func; }
    ~RegGuard() { g_reg_fun = nullptr; }
};

template <typename Type, typename... Args>
class ReactImpl : public Expression<Type, Args...> {
public:
    using ExprType = typename Expression<Type, Args...>::ExprType;
    using ValueType = typename Expression<Type, Args...>::ValueType;

    using Expression<Type, Args...>::Expression;

    template <typename T>
    void operator=(T&& other) {
        value(std::forward<T>(other));
    }

    decltype(auto) get() const { return this->getValue(); }

    auto getRaw() const { return this->getRawPtr(); }

    template <typename F, HasArguments... A>
    void set(F&& fun, A&&... args) {
        this->setSource(std::forward<F>(fun), std::forward<A>(args)...);
    }

    template <typename F>
    void set(F&& fun) {
        RegGuard guard([this](NodePtr node) { this->addObCb(node); });
        this->setSource(std::forward<F>(fun));
    }

    template <typename T>
        requires(Convertable<T, Type> && IsVarExpr<ExprType> && !ConstType<Type>)
    void value(T&& value) {
        this->updateValue(std::forward<T>(value));
        this->notify();
    }

    void addWeakRef() { weak_ref_count_.fetch_add(1, std::memory_order_relaxed); }

    void releaseWeakRef() {
        if (weak_ref_count_.fetch_sub(1, std::memory_order_relaxed) == 1) {
            ObserverGraph::GetInstance().removeNode(this->shared_from_this());
            if constexpr (HasField<ValueType>) {
                FieldGraph::GetInstance().deleteObj(this->getValue().getID());
            }
        }
    }

private:
    std::atomic<int> weak_ref_count_{0};
};

template <typename ReactType>
class React {
public:
    explicit React(std::shared_ptr<ReactType> ptr = nullptr) : weak_ptr_(ptr) {
        if (auto sp = weak_ptr_.lock()) {
            sp->addWeakRef();
        }
    }

    ~React() {
        if (auto sp = weak_ptr_.lock()) {
            sp->releaseWeakRef();
        }
    }

    React(const React& other) : weak_ptr_(other.weak_ptr_) {
        if (auto sp = weak_ptr_.lock()) {
            sp->addWeakRef();
        }
    }

    React(React&& other) noexcept : weak_ptr_(std::move(other.weak_ptr_)) {
        // No need to add weak ref, as ownership is transferred
    }

    React& operator=(const React& other) {
        if (this != &other) {
            if (auto sp = weak_ptr_.lock()) {
                sp->releaseWeakRef();
            }
            weak_ptr_ = other.weak_ptr_;
            if (auto sp = weak_ptr_.lock()) {
                sp->addWeakRef();
            }
        }
        return *this;
    }

    React& operator=(React&& other) noexcept {
        if (this != &other) {
            if (auto sp = weak_ptr_.lock()) {
                sp->releaseWeakRef();
            }
            weak_ptr_ = std::move(other.weak_ptr_);
            // No need to add weak ref, as ownership is transferred
        }
        return *this;
    }

    ReactType& operator*() const { return *getPtr(); }

    ReactType::ValueType* operator->() const { return getPtr()->getRaw(); }

    explicit operator bool() const { return !weak_ptr_.expired(); }

    decltype(auto) operator()() const {
        if (g_reg_fun) {
            std::invoke(g_reg_fun, getPtr());
        }
        return get();
    }

    std::shared_ptr<ReactType> getPtr() const {
        if (auto sp = weak_ptr_.lock()) {
            return sp;
        }
        throw std::runtime_error("Attempt to access expired React object");
    }

    decltype(auto) get() const
    // requires(IsDataReact<ReactType>)
    {
        return getPtr()->get();
    }

    template <typename F, typename... A>
    void reset(F&& fun, A&&... args) {
        getPtr()->set(std::forward<F>(fun), std::forward<A>(args)...);
    }

    template <typename T>
    void value(T&& value) {
        getPtr()->value(std::forward<T>(value));
    }

private:
    std::weak_ptr<ReactType> weak_ptr_;
};

template <typename T>
using Field = React<ReactImpl<std::decay_t<T>>>;

class FieldBase {
public:
    template <typename T>
    auto field(T&& value) {
        auto ptr = std::make_shared<ReactImpl<std::decay_t<T>>>(std::forward<T>(value));
        FieldGraph::GetInstance().addObj(id_, ptr);
        return React(ptr);
    }

    const UniqueID& getID() const { return id_; }

private:
    UniqueID id_;
};

template <typename T>
auto var(T&& value) {
    auto ptr = std::make_shared<ReactImpl<std::decay_t<T>>>(std::forward<T>(value));
    if constexpr (HasField<T>) {
        FieldGraph::GetInstance().bindField(value.getID(), ptr);
    }
    ObserverGraph::GetInstance().addNode(ptr);
    return React(ptr);
}

template <typename T>
auto constVar(T&& value) {
    auto ptr = std::make_shared<ReactImpl<const std::decay_t<T>>>(std::forward<T>(value));
    ObserverGraph::GetInstance().addNode(ptr);
    return React(ptr);
}

template <typename Fun, typename... Args>
auto calc(Fun&& fun, Args&&... args) {
    auto ptr = std::make_shared<ReactImpl<std::decay_t<Fun>, std::decay_t<Args>...>>();
    ObserverGraph::GetInstance().addNode(ptr);
    ptr->set(std::forward<Fun>(fun), std::forward<Args>(args)...);
    return React(ptr);
}

template <typename Fun, typename... Args>
auto action(Fun&& fun, Args&&... args) {
    return calc(std::forward<Fun>(fun), std::forward<Args>(args)...);
}

}  // namespace pyc::reaction