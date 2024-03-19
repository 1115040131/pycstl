#pragma once

#include <atomic>
#include <optional>

#include "common/noncopyable.h"

namespace pyc {
namespace concurrency {

template <typename T>
class RefCountStack : public Noncopyable {
private:
    struct RefNode;

    struct Node {
        T data;
        RefNode next{};
        std::atomic<int> dec_count{0};  // 减少的数量

        template <typename... Args>
        Node(const RefNode& next_, Args&&... args) : next(next_), data(std::forward<Args>(args)...) {}
    };

    struct RefNode {
        int ref_count;  // 引用计数
        Node* node_ptr;

        RefNode() : node_ptr(nullptr), ref_count(0) {}

        template <typename... Args>
        RefNode(const RefNode& next_, Args&&... args)
            : node_ptr(new Node(next_, std::forward<Args>(args)...)), ref_count(1) {}
    };

public:
    RefCountStack() = default;

    ~RefCountStack() {
        while (Pop()) {
        }
    }

    template <typename... Args>
    void Emplace(Args&&... args) {
        RefNode new_node(head_.load(), std::forward<Args>(args)...);
        while (!head_.compare_exchange_weak(new_node.node_ptr->next, new_node)) {
        }
    }

    void Push(const T& value) { Emplace(value); }

    void Push(T&& value) { Emplace(std::move(value)); }

    std::optional<T> Pop() {
        RefNode old_head = head_.load();
        for (;;) {
            // 1. 只要执行 Pop 就对引用计数 +1 并更新到 head 中
            RefNode new_head{};

            // 2
            do {
                new_head = old_head;
                ++new_head.ref_count;
            } while (!head_.compare_exchange_weak(old_head, new_head));
            old_head = new_head;

            // 3
            Node* node_ptr = old_head.node_ptr;
            if (node_ptr == nullptr) {
                return {};
            }

            // 4. 比较 head 和 old_head 是否相等
            if (head_.compare_exchange_strong(old_head, node_ptr->next)) {
                T value = std::move(node_ptr->data);

                // 5. 增加的数量
                int increase_count = old_head.ref_count - 2;
                if (node_ptr->dec_count.fetch_add(increase_count) == -increase_count) {
                    delete node_ptr;
                }
                return value;
            } else {
                if (node_ptr->dec_count.fetch_sub(1) == 1) {
                    delete node_ptr;
                }
            }
        }
        return {};
    }

private:
    std::atomic<RefNode> head_{};
};

}  // namespace concurrency
}  // namespace pyc
