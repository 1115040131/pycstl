#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>

#include "common/singleton.h"
#include "reaction/concept.h"
#include "reaction/utility.h"

namespace pyc::reaction {

inline thread_local NodeSet g_delay_list;

class ObserverGraph : public Singleton<ObserverGraph> {
public:
    void addNode(const NodePtr& node);

    void addObserver(const NodePtr& source, const NodePtr& target) {
        if (source == target) {
            throw std::runtime_error("Cannot add self as observer");
        }
        if (hasCycle(source, target)) {
            throw std::runtime_error("Cycle detected in observer graph");
        }

        hasRepeatDependencies(source, target);

        dependent_list_[source].insert(target);
        observer_list_.at(target).get().insert(source);
    }

    void removeNode(const NodePtr& node) {
        observer_list_.erase(node);
        dependent_list_.erase(node);
    }

    void setName(const NodePtr& node, const std::string& name) { name_list_.insert({node, name}); }

    std::string getName(const NodePtr& node) {
        auto it = name_list_.find(node);
        if (it != name_list_.end()) {
            return it->second;
        } else {
            return "";
        }
    }

private:
    bool hasCycle(const NodePtr& source, const NodePtr& target) {
        dependent_list_[source].insert(target);
        observer_list_.at(target).get().insert(source);

        NodeSet visited;
        NodeSet stack;

        bool cycle_found = dfs(source, visited, stack);

        dependent_list_[source].erase(target);
        observer_list_.at(target).get().erase(source);

        return cycle_found;
    }

    bool dfs(const NodePtr& node, NodeSet& visited, NodeSet& stack) {
        if (stack.contains(node)) {
            return true;
        }
        if (visited.contains(node)) {
            return false;
        }

        visited.insert(node);
        stack.insert(node);

        for (const auto& neighbor : dependent_list_[node]) {
            if (dfs(neighbor.lock(), visited, stack)) {
                return true;
            }
        }

        stack.erase(node);
        return false;
    }

    void hasRepeatDependencies(const NodePtr& source, const NodePtr& target) {
        // 获取目标节点的所有依赖节点
        NodeSet dependencies;
        collectDependencies(target, dependencies);

        // 检查源节点其他依赖节点中是否包含这些依赖节点
        NodeSet visited;
        for (auto& dependent : dependent_list_.at(source)) {
            checkDependency(source, dependent.lock(), dependencies, visited);
        }
    }

    void collectDependencies(const NodePtr& node, NodeMap& dependencies) {
        if (!node) {
            return;
        }

        dependencies[node]++;

        for (const auto& neighbour : dependent_list_.at(node)) {
            collectDependencies(neighbour.lock(), dependencies);
        }
    }

    void collectDependencies(const NodePtr& node, NodeSet& dependencies) {
        NodeMap target_dependencies;
        collectDependencies(node, target_dependencies);

        for (const auto& [dependent, count] : target_dependencies) {
            if (count == 1) {
                dependencies.insert(dependent);
            }
        }
    }

    void checkDependency(const NodePtr& source, const NodePtr& node, const NodeSet& dependencies,
                         NodeSet& visited) {
        if (!node || visited.contains(node)) {
            return;
        }
        visited.insert(node);

        if (dependencies.contains(node)) {
            if (repeat_list_.at(node).get().contains(source)) {
                repeat_list_.at(node).get()[source]++;
            } else {
                repeat_list_.at(node).get().emplace(source, 2);
            }
        }

        for (const auto& neighbour : dependent_list_.at(node)) {
            checkDependency(source, neighbour.lock(), dependencies, visited);
        }
    }

private:
    std::unordered_map<NodePtr, NodeSetRef> observer_list_;  // 节点的观察者列表
    std::unordered_map<NodePtr, NodeSet> dependent_list_;    // 节点的被观察者列表
    std::unordered_map<NodePtr, NodeMapRef> repeat_list_;    // 节点被重复依赖的列表
    std::unordered_map<NodePtr, std::string> name_list_;
};

class ObserverNode : public std::enable_shared_from_this<ObserverNode> {
    friend class ObserverGraph;

public:
    virtual ~ObserverNode() = default;

    virtual void valueChanged(bool changed = true) { this->notify(changed); }

    template <typename... Args>
    void updateObservers(Args&&... args) {
        auto shared_this = this->shared_from_this();
        (ObserverGraph::GetInstance().addObserver(shared_this, args), ...);
    }

    void notify(bool changed = true) {
        for (auto& [repeat, _] : repeats_) {
            g_delay_list.insert(repeat);
        }

        for (auto& observer : observers_) {
            if (g_delay_list.contains(observer)) {
                continue;
            }
            if (auto sp = observer.lock()) {
                sp->valueChanged(changed);
            }
        }

        if (!g_delay_list.empty()) {
            for (auto& [repeat, _] : repeats_) {
                g_delay_list.erase(repeat);
            }
            for (auto& [repeat, _] : repeats_) {
                if (auto sp = repeat.lock()) {
                    sp->valueChanged(changed);
                }
            }
        }
    }

private:
    NodeSet observers_;
    NodeMap repeats_;
};

class FieldGraph : public Singleton<FieldGraph> {
public:
    void addObj(const UniqueID& class_id, const NodePtr& obj) { field_map_[class_id].insert(obj); }

    void deleteObj(const UniqueID& class_id) { field_map_.erase(class_id); }

    void bindField(const UniqueID& class_id, const NodePtr& obj) {
        auto it = field_map_.find(class_id);
        if (it != field_map_.end()) {
            for (const auto& field_obj : it->second) {
                ObserverGraph::GetInstance().addObserver(obj, field_obj.lock());
            }
        }
    }

private:
    std::unordered_map<UniqueID, NodeSet> field_map_;
};

inline void ObserverGraph::addNode(const NodePtr& node) {
    observer_list_.emplace(node, std::ref(node->observers_));
    dependent_list_.emplace(node, NodeSet{});
    repeat_list_.emplace(node, std::ref(node->repeats_));
}

}  // namespace pyc::reaction