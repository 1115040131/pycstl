#pragma once

#include <memory>

#include "common/noncopyable.h"
#include "reaction/observer_node.h"

namespace pyc::reaction {

template <typename T>
class Resource : Noncopyable, public ObserverNode {
public:
    explicit Resource() = default;

    template <typename U>
    explicit Resource(U&& value) : ptr_(std::make_unique<T>(std::forward<U>(value))) {}

    T& getValue() const {
        if (!ptr_) {
            throw std::runtime_error("Resource is not initialized");
        }
        return *ptr_;
    }

    T* getRawPtr() const {
        if (!ptr_) {
            throw std::runtime_error("Resource is not initialized");
        }
        return ptr_.get();
    }

    template <typename U>
    void updateValue(U&& value) {
        if (!ptr_) {
            ptr_ = std::make_unique<T>(std::forward<U>(value));
        } else {
            *ptr_ = std::forward<U>(value);
        }
    }

private:
    std::unique_ptr<T> ptr_{nullptr};
};

struct VoidWrapper {};

template <>
class Resource<VoidWrapper> : public ObserverNode {
public:
    auto getValue() const { return VoidWrapper{}; }
};

}  // namespace pyc::reaction