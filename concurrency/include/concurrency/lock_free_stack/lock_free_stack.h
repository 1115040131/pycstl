#pragma once

#include <atomic>
#include <optional>

#include "concurrency/lock_free_stack/push_only_stack.h"

namespace pyc {
namespace concurrency {

template <typename T, typename Allocator = std::allocator<T>>
class LockFreeStack : public PushOnlyStack<T, Allocator> {
    using Node = typename PushOnlyStack<T, Allocator>::Node;  // 引入 Node 类型
    using PushOnlyStack<T, Allocator>::DeallocateNode;        // 引入 DeallocateNode 方法
    using PushOnlyStack<T, Allocator>::head_;                 // 引入 head_ 成员

public:
    using PushOnlyStack<T, Allocator>::PushOnlyStack;

    ~LockFreeStack() override { DeleteNodes(to_be_deleted_); }

    virtual std::optional<T> Pop() override {
        ++threads_in_pop_;
        Node* old_head = head_.load();
        while (old_head) {
            if (head_.compare_exchange_weak(old_head, old_head->next)) {
                T value = std::move(old_head->data);
                TryReclaim(old_head);
                return value;
            }
        }
        return {};
    }

private:
    /// @brief 延迟回收节点
    void TryReclaim(Node* old_head) {
        // 1 原子变量判断仅有一个线程进入
        if (threads_in_pop_ == 1) {
            // 2 取出待删列表
            Node* nodes_to_delete = to_be_deleted_.exchange(nullptr);
            // 3 更新原子变量获取准确状态
            if (!--threads_in_pop_) {
                // 4 如果唯一调用则将待删列表删除
                DeleteNodes(nodes_to_delete);
            } else if (nodes_to_delete) {
                // 5 有其他线程在 pop 且待删列表不为空, 更新待删列表首节点
                ChainPendingNodes(nodes_to_delete);
            }
            DeallocateNode(old_head);
        } else {
            // 多个线程 Pop 竞争, 将其放入待删列表
            ChainPendingNode(old_head);
            --threads_in_pop_;
        }
    }

    void DeleteNodes(Node* node) {
        while (node) {
            Node* next = node->next;
            DeallocateNode(node);
            node = next;
        }
    }

    /// @brief 将 first 到 last 的链表添加到待删列表
    void ChainPendingNodes(Node* first, Node* last) {
        // 1. 将 last 的 next 更新为待删列表的首节点
        last->next = to_be_deleted_;
        // 2. 将待删列表首节点更新为 first 节点
        while (!to_be_deleted_.compare_exchange_weak(last->next, first)) {
        }
    }

    void ChainPendingNode(Node* node) { ChainPendingNodes(node, node); }

    void ChainPendingNodes(Node* first) {
        Node* last = first;
        while (last->next) {
            last = last->next;
        }
        ChainPendingNodes(first, last);
    }

private:
    std::atomic<int> threads_in_pop_;   // 正在执行 pop 的线程数
    std::atomic<Node*> to_be_deleted_;  // 待删列表首节点
};

}  // namespace concurrency
}  // namespace pyc
