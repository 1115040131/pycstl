#pragma once

#include <memory>

#include "common/noncopyable.h"

namespace pyc::reaction {

template <typename T>
class Resource : Noncopyable {
public:
    explicit Resource() = default;

    template <typename U>
    explicit Resource(U&& value) : ptr_(std::make_unique<T>(std::forward<U>(value))) {}

    const T& getValue() const {
        if (!ptr_) {
            throw std::runtime_error("Resource is not initialized");
        }
        return *ptr_;
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

}  // namespace pyc::reaction