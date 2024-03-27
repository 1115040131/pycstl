#pragma once

#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>

#include "common/noncopyable.h"

namespace pyc {
namespace concurrency {

/// @brief 链表实现的线程安全队列, 可以同时 Push 和 Pop
template <typename T, typename Allocator = std::allocator<T>>
class ThreadSafeQueue : public Noncopyable {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;
    };

    using AllocNode = std::allocator_traits<Allocator>::template rebind_alloc<Node>;

    Node* NewNode() { return AllocNode{alloc_}.allocate(1); }

    void DeleteNode(Node* node) noexcept { AllocNode{alloc_}.deallocate(node, 1); }

public:
    explicit ThreadSafeQueue() { head_ = tail_ = NewNode(); }

    ~ThreadSafeQueue() {
        while (head_ != tail_) {
            std::destroy_at(&head_->data);
            auto next = head_;
            DeleteNode(head_);
            head_ = next;
        }
        DeleteNode(head_);
    }

    template <typename... Args>
    void Emplace(Args&&... args) {
        auto new_tail = NewNode();
        std::lock_guard<std::mutex> lock(tail_mtx_);
        std::construct_at(&tail_->data, std::forward<Args>(args)...);
        new_tail->prev = tail_;
        tail_->next = new_tail;
        tail_ = new_tail;
        data_cond_.notify_one();
    }

    void Push(const T& value) { Emplace(value); }

    void Push(T&& value) { Emplace(std::move(value)); }

    T WaitAndPop() {
        std::unique_lock<std::mutex> lock(head_mtx_);
        data_cond_.wait(lock, [this] { return head_ != tail_; });
        return Pop();
    }

    std::optional<T> TryPop() {
        std::lock_guard<std::mutex> lock(head_mtx_);
        if (head_ == tail_) {
            return {};
        }
        return Pop();
    }

    std::optional<T> TrySteal() {
        std::scoped_lock<std::mutex> guard(head_mtx_, tail_mtx_);
        if (head_ == tail_) {
            return {};
        }
        T value = std::move(tail_->prev->data);
        tail_ = tail_->prev;
        DeleteNode(tail_->next);
        tail_->next = nullptr;
        return value;
    }

    bool Empty() const {
        std::lock_guard<std::mutex> lock(head_mtx_);
        return head_ == tail_;
    }

private:
    T Pop() {
        T value = std::move(head_->data);
        auto next = head_->next;
        DeleteNode(head_);
        head_ = next;
        return value;
    }

private:
    mutable std::mutex head_mtx_;
    mutable std::mutex tail_mtx_;
    Node* head_;
    Node* tail_;
    std::condition_variable data_cond_;
    [[no_unique_address]] Allocator alloc_;
};

}  // namespace concurrency
}  // namespace pyc
