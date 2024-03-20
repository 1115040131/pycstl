#pragma once

#include <memory>
#include <mutex>

#include "common/noncopyable.h"

namespace pyc {
namespace concurrency {

template <typename T>
class ThreadSafeList : public Noncopyable {
private:
    struct Node {
        std::mutex mtx;
        std::shared_ptr<T> data;
        std::unique_ptr<Node> next;

        Node() = default;
        Node(const T& value) : data(std::make_shared<T>(value)) {}
    };

public:
    ThreadSafeList() = default;

    ~ThreadSafeList() {
        RemoveIf([](const T&) { return true; });
    }

    template <typename Predicate>
    void RemoveIf(Predicate p) {
        Node* curr = &head_;
        std::unique_lock<std::mutex> lock(head_.mtx);
        while (Node* const next = curr->next.get()) {
            std::unique_lock<std::mutex> next_lock(next->mtx);
            if (p(*next->data)) {
                std::unique_ptr<Node> old_next = std::move(curr->next);
                curr->next = std::move(old_next->next);
                // old_next 持有被删除节点的所有权, 当 old_next 离开作用域会销毁 next_lock 所持有的互斥锁
                // 为防止当 next_lock 离开作用域时尝试解锁一个已经被销毁的互斥锁在这里提前解锁
                next_lock.unlock();
            } else {
                lock.unlock();
                curr = next;
                lock = std::move(next_lock);
            }
        }
    }

    void PushFront(const T& value) {
        auto new_node = std::make_unique<Node>(value);
        std::lock_guard<std::mutex> lock(head_.mtx);
        new_node->next = std::move(head_.next);
        head_.next = std::move(new_node);
    }

    template <typename Predicate>
    std::shared_ptr<T> FindFirstIf(Predicate p) {
        Node* curr = &head_;
        std::unique_lock<std::mutex> lock(head_.mtx);
        while (Node* const next = curr->next.get()) {
            std::unique_lock<std::mutex> next_lock(next->mtx);
            lock.unlock();
            if (p(*next->data)) {
                return next->data;
            }
            curr = next;
            lock = std::move(next_lock);
        }
        return {};
    }

    template <typename Function>
    void ForEach(Function f) {
        Node* curr = &head_;
        std::unique_lock<std::mutex> lock(head_.mtx);
        while (const std::unique_ptr<Node>& next = curr->next) {
            std::unique_lock<std::mutex> next_lock(next->mtx);
            lock.unlock();
            f(*next->data);
            curr = next.get();
            lock = std::move(next_lock);
        }
    }

private:
    Node head_;
};

}  // namespace concurrency
}  // namespace pyc
