#pragma once

#include <atomic>
#include <optional>

#include "common/noncopyable.h"

namespace pyc {
namespace concurrency {

template <typename T, typename Allocator = std::allocator<T>>
class PushOnlyStack : public Noncopyable {
protected:
    struct Node {
        T data;
        Node* next;

        Node() = default;
        Node(const T& _data) : data(_data) {}
    };

    using AllocNode = std::allocator_traits<Allocator>::template rebind_alloc<Node>;

    Node* AllocateNode() { return AllocNode{alloc_}.allocate(1); }

    void DeallocateNode(Node* node) noexcept { AllocNode{alloc_}.deallocate(node, 1); }

public:
    PushOnlyStack() = default;

    virtual ~PushOnlyStack() = default;

    template <typename... Args>
    void Emplace(Args&&... args) {
        Node* new_node = AllocateNode();
        std::construct_at(&new_node->data, std::forward<Args>(args)...);
        new_node->next = head_.load();
        while (!head_.compare_exchange_weak(new_node->next, new_node)) {
        }
    }

    void Push(const T& value) { Emplace(value); }

    void Push(T&& value) { Emplace(std::move(value)); }

    virtual std::optional<T> Pop() = 0;

protected:
    std::atomic<Node*> head_;
    [[no_unique_address]] Allocator alloc_;
};

}  // namespace concurrency
}  // namespace pyc
