#pragma once

#include <atomic>
#include <optional>

#include "concurrency/lock_free_stack/push_only_stack.h"

namespace pyc {
namespace concurrency {

struct HazardPointer {
    std::atomic<std::thread::id> id;
    std::atomic<void*> pointer;
};

constexpr std::size_t kMaxHazardPointer = 100;
HazardPointer hazard_pointers[kMaxHazardPointer];

class HpOwner : public Noncopyable {
public:
    HpOwner() : hp(nullptr) {
        for (std::size_t i = 0; i < kMaxHazardPointer; i++) {
            std::thread::id old_id;
            if (hazard_pointers[i].id.compare_exchange_strong(old_id, std::this_thread::get_id())) {
                hp = &hazard_pointers[i];
                break;
            }
        }

        if (!hp) {
            throw std::runtime_error("No hazard pointer available");
        }
    }

    ~HpOwner() {
        hp->pointer.store(nullptr);
        hp->id.store(std::thread::id());
    }

    std::atomic<void*>& GetPointer() { return hp->pointer; }

private:
    HazardPointer* hp;
};

template <typename T, typename Allocator = std::allocator<T>>
class HazardPointerStack : public PushOnlyStack<T, Allocator> {
    using Node = typename PushOnlyStack<T, Allocator>::Node;  // 引入 Node 类型
    using PushOnlyStack<T, Allocator>::DeallocateNode;        // 引入 DeallocateNode 方法
    using PushOnlyStack<T, Allocator>::head_;                 // 引入 head_ 成员

public:
    using PushOnlyStack<T, Allocator>::PushOnlyStack;

    ~HazardPointerStack() override = default;

    virtual std::optional<T> Pop() override {
        // 1. 从风险列表中获取一个节点给当前线程
        std::atomic<void*>& hp = GetHazardPointerForCurrentThread();
        Node* old_head = head_.load();
        while (old_head) {
            Node* temp;
            do {
                temp = old_head;
                hp.store(old_head);
                old_head = head_.load();
            }  // 2. 如果 old_head 和 temp 不等说明 head 被其他线程更新了，需重试
            while (old_head != temp);

            // 3. 将当前 head 更新为 old_head->next, 如不满足则重试
            if (head_.compare_exchange_weak(old_head, old_head->next)) {
                // 4一旦更新了 head_ 指针，便将风险指针清零
                hp.store(nullptr);
                auto result = std::move(old_head->data);
                // 5 删除旧有的头节点之前，先核查它是否正被风险指针所指涉
                if (OutstandingHazardPointersFor(old_head)) {
                    // 6 延迟删除
                    ReclaimLater(old_head);
                } else {
                    // 7 删除头部节点
                    DeallocateNode(old_head);
                }
                // 8 删除没有风险的节点
                DeleteNodesWithNoHazards();
                return result;
            }
        }
        return {};
    }

private:
    struct data_to_reclaim {
        Node* data;
        data_to_reclaim* next;
        data_to_reclaim(Node* p) : data(p), next(nullptr) {}
        ~data_to_reclaim() { delete data; }
    };

    std::atomic<void*>& GetHazardPointerForCurrentThread() {
        thread_local static HpOwner hazard;
        return hazard.GetPointer();
    }

    bool OutstandingHazardPointersFor(void* p) {
        for (std::size_t i = 0; i < kMaxHazardPointer; i++) {
            if (hazard_pointers[i].pointer.load() == p) {
                return true;
            }
        }
        return false;
    }

    void ReclaimLater(Node* old_head) { AddToReclaimList(new data_to_reclaim(old_head)); }

    void AddToReclaimList(data_to_reclaim* reclaim_node) {
        reclaim_node->next = nodes_to_reclaim.load();
        while (!nodes_to_reclaim.compare_exchange_weak(reclaim_node->next, reclaim_node)) {
        }
    }

    void DeleteNodesWithNoHazards() {
        data_to_reclaim* current = nodes_to_reclaim.exchange(nullptr);
        while (current) {
            data_to_reclaim* const next = current->next;
            if (!OutstandingHazardPointersFor(current->data)) {
                delete current;
            } else {
                AddToReclaimList(current);
            }
            current = next;
        }
    }

private:
    std::atomic<data_to_reclaim*> nodes_to_reclaim;
};

}  // namespace concurrency
}  // namespace pyc
