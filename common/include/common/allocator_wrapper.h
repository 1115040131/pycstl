#pragma once

#include <memory>

namespace pyc {

// AllocatorWrapper 封装了与分配器相关的操作
template <typename T, typename Allocator = std::allocator<T>>
class AllocatorWrapper {
public:
    explicit AllocatorWrapper(const Allocator& alloc = Allocator()) : alloc_(RebindAlloc(alloc)) {}

    // 分配内存，用于 T 类型
    T* Allocate(std::size_t n = 1) { return RebindTraits::allocate(alloc_, n); }

    // 释放内存，用于 T 类型
    void Deallocate(T* ptr, std::size_t n = 1) noexcept { RebindTraits::deallocate(alloc_, ptr, n); }

protected:
    // 使用 allocator_traits 来获得正确的分配器类型
    using AllocTraits = std::allocator_traits<Allocator>;
    // 用来重绑定原始分配器到 T 类型
    using RebindAlloc = typename AllocTraits::template rebind_alloc<T>;
    // 重绑定后的分配器特性
    using RebindTraits = std::allocator_traits<RebindAlloc>;

    // 分配器实例（已重绑定到 T 类型）
    [[no_unique_address]] RebindAlloc alloc_;
};

}  // namespace pyc
