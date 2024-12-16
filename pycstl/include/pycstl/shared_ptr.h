#pragma once

#include <atomic>

#include "pycstl/unique_ptr.h"

namespace pycstl {

struct _SpCounter {
    explicit _SpCounter() noexcept : ref_count_(1) {}

    _SpCounter(_SpCounter&&) = delete;

    virtual ~_SpCounter() noexcept = default;

    void _M_incref() noexcept { ref_count_.fetch_add(1, std::memory_order_relaxed); }

    void _M_decref() noexcept {
        if (ref_count_.fetch_sub(1, std::memory_order_relaxed) == 1) {
            delete this;
        }
    }

    long _M_cntref() const noexcept { return ref_count_.load(std::memory_order_relaxed); }

    std::atomic<long> ref_count_;
};

template <typename T, typename Deleter>
struct _SpCounterImpl : public _SpCounter {
    explicit _SpCounterImpl(T* ptr) noexcept : data_(ptr) {}

    explicit _SpCounterImpl(T* ptr, Deleter deleter) noexcept : data_(ptr), deleter_(std::move(deleter)) {}

    ~_SpCounterImpl() noexcept override { deleter_(data_); }

    T* data_;
    [[no_unique_address]] Deleter deleter_;
};

template <typename T>
class SharedPtr {
    template <typename>
    friend class SharedPtr;

    explicit SharedPtr(T* ptr, _SpCounter* owner) noexcept : ptr_(ptr), owner_(owner) {}

public:
    using element_type = T;

    // (1) (2)
    SharedPtr(std::nullptr_t = nullptr) noexcept : owner_(nullptr) {}

    // (3)
    template <typename Y, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    explicit SharedPtr(Y* ptr) : ptr_(ptr), owner_(new _SpCounterImpl<Y, DefaultDeleter<Y>>(ptr)) {
        _S_setupEnableSharedFromThis(ptr, owner_);
    }

    // (4)
    template <typename Y, typename Deleter, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    explicit SharedPtr(Y* ptr, Deleter deleter)
        : ptr_(ptr), owner_(new _SpCounterImpl<Y, Deleter>(ptr, std::move(deleter))) {
        _S_setupEnableSharedFromThis(ptr, owner_);
    }

    // (8)
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& that, T* ptr) noexcept : ptr_(ptr), owner_(that.owner_) {
        owner_->_M_incref();
    }

    template <typename Y>
    SharedPtr(SharedPtr<Y>&& that, T* ptr) noexcept : ptr_(ptr), owner_(that.owner_) {
        that.ptr_ = nullptr;
        that.owner_ = nullptr;
    }

    // (9)
    SharedPtr(const SharedPtr& that) noexcept : ptr_(that.ptr_), owner_(that.owner_) {
        if (owner_) {
            owner_->_M_incref();
        }
    }

    template <typename Y, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    SharedPtr(const SharedPtr<Y>& that) noexcept : ptr_(that.ptr_), owner_(that.owner_) {
        if (owner_) {
            owner_->_M_incref();
        }
    }

    // (10)
    SharedPtr(SharedPtr&& that) noexcept : ptr_(that.ptr_), owner_(that.owner_) {
        that.ptr_ = nullptr;
        that.owner_ = nullptr;
    }

    template <typename Y, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    SharedPtr(SharedPtr<Y>&& that) noexcept : ptr_(that.ptr_), owner_(that.owner_) {
        that.ptr_ = nullptr;
        that.owner_ = nullptr;
    }

    // (13)
    template <typename Y, typename Deleter, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    explicit SharedPtr(UniquePtr<Y, Deleter>&& ptr) : SharedPtr(ptr.release(), Deleter{}) {}

    template <class Y>
    inline friend SharedPtr<Y> _S_makeSharedFused(Y* ptr, _SpCounter* owner) noexcept;

    template <typename Y, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    SharedPtr& operator=(const SharedPtr<Y>& that) noexcept {
        if (this == &that) {
            return *this;
        }
        if (owner_) {
            owner_->_M_decref();
        }
        ptr_ = that.ptr_;
        owner_ = that.owner_;
        if (owner_) {
            owner_->_M_incref();
        }
        return *this;
    }

    template <typename Y, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    SharedPtr& operator=(SharedPtr<Y>&& that) noexcept {
        if (this == &that) {
            return *this;
        }
        if (owner_) {
            owner_->_M_decref();
        }
        ptr_ = that.ptr_;
        owner_ = that.owner_;
        that.ptr_ = nullptr;
        that.owner_ = nullptr;
        return *this;
    }

    ~SharedPtr() noexcept {
        if (owner_) {
            owner_->_M_decref();
        }
    }

#pragma region 修改器

    void reset() noexcept {
        if (owner_) {
            owner_->_M_decref();
        }
        ptr_ = nullptr;
        owner_ = nullptr;
    }

    template <typename Y, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    void reset(Y* ptr) {
        if (owner_) {
            owner_->_M_decref();
        }
        ptr_ = nullptr;
        owner_ = nullptr;
        ptr_ = ptr;
        owner_ = new _SpCounterImpl<Y, DefaultDeleter<Y>>(ptr);
        _S_setupEnableSharedFromThis(ptr, owner_);
    }

    template <typename Y, typename Deleter, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    void reset(Y* ptr, Deleter deleter) {
        if (owner_) {
            owner_->_M_decref();
        }
        ptr_ = nullptr;
        owner_ = nullptr;
        ptr_ = ptr;
        owner_ = new _SpCounterImpl<Y, Deleter>(ptr, deleter);
        _S_setupEnableSharedFromThis(ptr, owner_);
    }

    void swap(SharedPtr& that) noexcept {
        std::swap(ptr_, that.ptr_);
        std::swap(owner_, that.owner_);
    }

#pragma endregion

#pragma region 观察器

    T* get() const noexcept { return ptr_; }

    std::add_lvalue_reference_t<T> operator*() const noexcept { return *ptr_; }

    T* operator->() const noexcept { return ptr_; }

    long use_count() const noexcept { return owner_ ? owner_->_M_cntref() : 0; }

    template <class Y, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    bool owner_before(const SharedPtr<Y>& that) const noexcept {
        return owner_ < that.owner_;
    }

    template <class Y, std::enable_if_t<std::is_convertible_v<Y*, T*>, int> = 0>
    bool owner_equal(const SharedPtr<Y>& that) const noexcept {
        return owner_ == that.owner_;
    }

#pragma endregion

private:
    T* ptr_;
    _SpCounter* owner_;
};

template <class T>
inline SharedPtr<T> _S_makeSharedFused(T* ptr, _SpCounter* owner) noexcept {
    return SharedPtr<T>(ptr, owner);
}

template <typename T, typename... Args, std::enable_if_t<!std::is_unbounded_array_v<T>, int> = 0>
SharedPtr<T> MakeShared(Args&&... args) {
    return SharedPtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T, std::enable_if_t<!std::is_unbounded_array_v<T>, int> = 0>
SharedPtr<T> MakeSharedForOverwrite() {
    return SharedPtr<T>(new T);
}

template <typename T, std::enable_if_t<std::is_unbounded_array_v<T>, int> = 0>
SharedPtr<T> MakeShared(std::size_t n) {
    return SharedPtr<T>(new std::remove_extent_t<T>[n]());
}

template <typename T, std::enable_if_t<std::is_unbounded_array_v<T>, int> = 0>
SharedPtr<T> MakeSharedForOverwrite(std::size_t n) {
    return SharedPtr<T>(new std::remove_extent_t<T>[n]);
}

template <typename T>
struct SharedPtr<T[]> : SharedPtr<T> {
    using SharedPtr<T>::SharedPtr;

    std::add_lvalue_reference_t<T> operator[](std::size_t i) const noexcept { return this->ptr_[i]; }
};

template <typename T, typename U>
SharedPtr<T> StaticPointerCast(const SharedPtr<U>& ptr) {
    return SharedPtr<T>(ptr, static_cast<T*>(ptr.get()));
}

template <typename T, typename U>
SharedPtr<T> ConstPointerCast(const SharedPtr<U>& ptr) {
    return SharedPtr<T>(ptr, const_cast<T*>(ptr.get()));
}

template <typename T, typename U>
SharedPtr<T> DynamicPointerCast(const SharedPtr<U>& ptr) {
    if (auto* p = dynamic_cast<T*>(ptr.get())) {
        return SharedPtr<T>(ptr, p);
    }
    return SharedPtr<T>();
}

template <typename T, typename U>
SharedPtr<T> ReinterpretPointerCast(const SharedPtr<U>& ptr) {
    return SharedPtr<T>(ptr, reinterpret_cast<T*>(ptr.get()));
}

template <typename Derived>
class EnableSharedFromThis {
protected:
    SharedPtr<Derived> shared_from_this() {
        static_assert(std::is_base_of_v<EnableSharedFromThis, Derived>,
                      "Derived must inherit from EnableSharedFromThis");
        if (!owner_) {
            throw std::bad_weak_ptr();
        }
        owner_->_M_incref();
        return _S_makeSharedFused(static_cast<Derived*>(this), owner_);
    }

    SharedPtr<std::add_const_t<Derived>> shared_from_this() const {
        static_assert(std::is_base_of_v<EnableSharedFromThis, Derived>,
                      "Derived must inherit from EnableSharedFromThis");
        if (!owner_) {
            throw std::bad_weak_ptr();
        }
        owner_->_M_incref();
        return _S_makeSharedFused(static_cast<std::add_const_t<Derived>*>(this), owner_);
    }

    template <class U>
    inline friend void _S_setEnableSharedFromThisOwner(EnableSharedFromThis<U>*, _SpCounter*);

private:
    _SpCounter* owner_;
};

template <class U>
inline void _S_setEnableSharedFromThisOwner(EnableSharedFromThis<U>* ptr, _SpCounter* owner) {
    ptr->owner_ = owner;
}

template <class T, std::enable_if_t<std::is_base_of_v<EnableSharedFromThis<T>, T>, int> = 0>
void _S_setupEnableSharedFromThis(T* ptr, _SpCounter* owner) {
    _S_setEnableSharedFromThisOwner(static_cast<EnableSharedFromThis<T>*>(ptr), owner);
}

template <class T, std::enable_if_t<!std::is_base_of_v<EnableSharedFromThis<T>, T>, int> = 0>
void _S_setupEnableSharedFromThis(T*, _SpCounter*) {}

}  // namespace pycstl
