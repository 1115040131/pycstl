#pragma once

#include <atomic>
#include <memory>

#include "common/noncopyable.h"
#include "concurrency/memory_order.h"

namespace pyc {
namespace concurrency {

template <typename T, bool UseMemoryOrder>
class LockFreeQueue : public Noncopyable {
    using MemoryOrder = std::conditional_t<UseMemoryOrder, StdMemoryOrder, DefaultMemoryOrder>;

    struct NodeCounter {
        unsigned internal_count : 30;
        unsigned external_count : 2;
    };

    struct Node;

    struct CountedNodePtr {
        int external_count = 0;
        Node* ptr = nullptr;
    };

    struct Node {
        std::atomic<T*> data;
        std::atomic<NodeCounter> count;
        std::atomic<CountedNodePtr> next;

        Node(int external_count = 2) {
            count.store(NodeCounter{0, external_count});
            next.store(CountedNodePtr{0, nullptr});
        }

        void ReleaseRef() {
            NodeCounter old_counter = count.load(MemoryOrder::memory_order_relaxed);
            NodeCounter new_counter;
            do {
                new_counter = old_counter;
                --new_counter.internal_count;
            } while (!count.compare_exchange_strong(old_counter, new_counter, MemoryOrder::memory_order_acquire,
                                                    MemoryOrder::memory_order_relaxed));
            if (new_counter.internal_count == 0 && new_counter.external_count == 0) {
                delete this;
            }
        }
    };

public:
    LockFreeQueue() {
        CountedNodePtr new_next{1, new Node()};
        tail_.store(new_next, MemoryOrder::memory_order_relaxed);
        head_.store(new_next, MemoryOrder::memory_order_relaxed);
    }

    ~LockFreeQueue() {
        while (Pop()) {
        }
        CountedNodePtr old_head = head_.load();
        delete old_head.ptr;
    }

    template <typename... Args>
    void Emplace(Args&&... args) {
        std::unique_ptr<T> new_data(new T(std::forward<Args>(args)...));
        CountedNodePtr new_next{1, new Node()};
        CountedNodePtr old_tail = tail_.load();
        for (;;) {
            IncreaseExternalCount(tail_, old_tail);
            T* old_data = nullptr;
            if (old_tail.ptr->data.compare_exchange_strong(old_data, new_data.get())) {
                CountedNodePtr old_next{};
                if (!old_tail.ptr->next.compare_exchange_strong(old_next, new_next)) {
                    delete new_next.ptr;
                    new_next = old_next;
                }
                SetNewTail(old_tail, new_next);
                new_data.release();
                break;
            } else {
                CountedNodePtr old_next{};
                if (old_tail.ptr->next.compare_exchange_strong(old_next, new_next)) {
                    old_next = new_next;
                    new_next.ptr = new Node();
                }
                SetNewTail(old_tail, old_next);
            }
        }
    }

    void Push(const T& data) { Emplace(data); }

    void Push(T&& data) { Emplace(data); }

    std::unique_ptr<T> Pop() {
        CountedNodePtr old_head = head_.load(MemoryOrder::memory_order_relaxed);
        for (;;) {
            IncreaseExternalCount(head_, old_head);
            Node* const ptr = old_head.ptr;
            if (ptr == tail_.load().ptr) {
                ptr->ReleaseRef();
                return {};
            }
            CountedNodePtr next = ptr->next.load();
            if (head_.compare_exchange_strong(old_head, next)) {
                T* const res = ptr->data.exchange(nullptr);
                FreeExternalCount(old_head);
                return std::unique_ptr<T>(res);
            }
            ptr->ReleaseRef();
        }
    }

private:
    static void IncreaseExternalCount(std::atomic<CountedNodePtr>& counter, CountedNodePtr& old_counter) {
        CountedNodePtr new_counter;
        do {
            new_counter = old_counter;
            ++new_counter.external_count;
        } while (!counter.compare_exchange_strong(old_counter, new_counter, MemoryOrder::memory_order_acquire,
                                                  MemoryOrder::memory_order_relaxed));
        old_counter.external_count = new_counter.external_count;
    }

    static void FreeExternalCount(CountedNodePtr& old_Node_ptr) {
        Node* const ptr = old_Node_ptr.ptr;
        const int count_increase = old_Node_ptr.external_count - 2;
        NodeCounter old_counter = ptr->count.load(MemoryOrder::memory_order_relaxed);
        NodeCounter new_counter;
        do {
            new_counter = old_counter;
            --new_counter.external_count;
            new_counter.internal_count += count_increase;
        } while (!ptr->count.compare_exchange_strong(old_counter, new_counter, MemoryOrder::memory_order_acquire,
                                                     MemoryOrder::memory_order_relaxed));
        if (new_counter.internal_count == 0 && new_counter.external_count == 0) {
            delete ptr;
        }
    }

    void SetNewTail(CountedNodePtr& old_tail, const CountedNodePtr& new_tail) {
        Node* const current_tail_ptr = old_tail.ptr;
        while (!tail_.compare_exchange_weak(old_tail, new_tail) && old_tail.ptr == current_tail_ptr) {
        }
        if (old_tail.ptr == current_tail_ptr) {
            FreeExternalCount(old_tail);
        } else {
            current_tail_ptr->ReleaseRef();
        }
    }

private:
    std::atomic<CountedNodePtr> head_;
    std::atomic<CountedNodePtr> tail_;
};

}  // namespace concurrency
}  // namespace pyc
