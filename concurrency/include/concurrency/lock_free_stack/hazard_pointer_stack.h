#pragma once

#include <atomic>
#include <optional>
#include <thread>

#include "common/allocator_wrapper.h"
#include "common/noncopyable.h"

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
class HazardPointerStack : public Noncopyable {
private:
    struct Node {
        T data;
        Node* next;

        Node() = default;
        Node(const T& _data) : data(_data) {}
    };

public:
    HazardPointerStack() = default;

    ~HazardPointerStack() {
        DeleteNodesWithNoHazards();
        while (Pop()) {
        }
    }

    template <typename... Args>
    void Emplace(Args&&... args) {
        Node* new_node = alloc_.Allocate();
        std::construct_at(&new_node->data, std::forward<Args>(args)...);
        new_node->next = head_.load();
        while (!head_.compare_exchange_weak(new_node->next, new_node)) {
        }
    }

    void Push(const T& value) { Emplace(value); }

    void Push(T&& value) { Emplace(std::move(value)); }

    std::optional<T> Pop() {
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
                    alloc_.Deallocate(old_head);
                }
                // 8 删除没有风险的节点
                DeleteNodesWithNoHazards();
                return result;
            }
        }
        return {};
    }

private:
    struct DataToReclaim {
        Node* data;
        DataToReclaim* next;
        DataToReclaim(Node* p) : data(p), next(nullptr) {}
        ~DataToReclaim() = default;  // 资源由 HazardPointerStack 释放
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

    void ReclaimLater(Node* old_head) {
        DataToReclaim* reclaim_node = reclaim_alloc_.Allocate();
        std::construct_at(&reclaim_node->data, old_head);
        AddToReclaimList(reclaim_node);
    }

    void AddToReclaimList(DataToReclaim* reclaim_node) {
        reclaim_node->next = nodes_to_reclaim.load();
        while (!nodes_to_reclaim.compare_exchange_weak(reclaim_node->next, reclaim_node)) {
        }
    }

    void DeleteNodesWithNoHazards() {
        DataToReclaim* current = nodes_to_reclaim.exchange(nullptr);
        while (current) {
            DataToReclaim* const next = current->next;
            if (!OutstandingHazardPointersFor(current->data)) {
                alloc_.Deallocate(current->data);
                reclaim_alloc_.Deallocate(current);
            } else {
                AddToReclaimList(current);
            }
            current = next;
        }
    }

private:
    std::atomic<Node*> head_{nullptr};
    std::atomic<DataToReclaim*> nodes_to_reclaim;
    [[no_unique_address]] AllocatorWrapper<Node, Allocator> alloc_;
    [[no_unique_address]] AllocatorWrapper<DataToReclaim, Allocator> reclaim_alloc_;
};

}  // namespace concurrency
}  // namespace pyc
