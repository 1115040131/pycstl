#pragma once

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "common/singleton.h"
#include "reaction/concept.h"
#include "reaction/utility.h"

namespace pyc::reaction {

using NodeSet = std::unordered_set<NodePtr>;
using NodeSetRef = std::reference_wrapper<NodeSet>;

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

        dependent_list_[source].insert(target);
        observer_list_.at(target).get().insert(source);
    }

    void removeNode(const NodePtr& node) {
        observer_list_.erase(node);
        dependent_list_.erase(node);
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
            if (dfs(neighbor, visited, stack)) {
                return true;
            }
        }

        stack.erase(node);
        return false;
    }

private:
    std::unordered_map<NodePtr, NodeSetRef> observer_list_;
    std::unordered_map<NodePtr, NodeSet> dependent_list_;
};

#ifdef USE_FUNCTION
class ObserverNode : public std::enable_shared_from_this<ObserverNode> {
public:
    void addObserver(const std::function<void()>& f) { observers_.emplace_back(f); }

    template <typename... Args>
    void updateObservers(const std::function<void()>& f, Args&&... args) {
        (..., args->addObserver(f));
    }

    void notify() {
        for (auto& observer : observers_) {
            observer();
        }
    }

private:
    std::vector<std::function<void()>> observers_;
};
#else
class ObserverNode : public std::enable_shared_from_this<ObserverNode> {
    friend class ObserverGraph;

public:
    virtual ~ObserverNode() = default;

    virtual void valueChanged() { this->notify(); }

    template <typename... Args>
    void updateObservers(Args&&... args) {
        auto shared_this = this->shared_from_this();
        (ObserverGraph::GetInstance().addObserver(shared_this, args), ...);
    }

    void notify() {
        for (auto& observer : observers_) {
            observer->valueChanged();
        }
    }

private:
    NodeSet observers_;
};
#endif

class FieldGraph : public Singleton<FieldGraph> {
public:
    void addObj(const UniqueID& class_id, const NodePtr& obj) { field_map_[class_id].insert(obj); }

    void deleteObj(const UniqueID& class_id) { field_map_.erase(class_id); }

    void bindField(const UniqueID& class_id, const NodePtr& obj) {
        auto it = field_map_.find(class_id);
        if (it != field_map_.end()) {
            for (const auto& field_obj : it->second) {
#ifdef USE_FUNCTION
                field_obj->addObserver([obj]() { obj->notify(); });
#else
                ObserverGraph::GetInstance().addObserver(obj, field_obj);
#endif
            }
        }
    }

private:
    std::unordered_map<UniqueID, NodeSet> field_map_;
};

inline void ObserverGraph::addNode(const NodePtr& node) {
    observer_list_.emplace(node, std::ref(node->observers_));
    dependent_list_.emplace(node, NodeSet{});
}

}  // namespace pyc::reaction