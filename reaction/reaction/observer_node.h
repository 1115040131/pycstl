#pragma once

#ifdef USE_FUNCTION
#include <functional>
#endif
#include <unordered_set>
#include <vector>

#include "common/singleton.h"
#include "reaction/concept.h"

namespace pyc::reaction {

#ifdef USE_FUNCTION
class ObserverNode : public std::enable_shared_from_this<ObserverNode> {
public:
    void addObserver(const std::function<void()>& f) { observers_.emplace_back(f); }

    template <typename... Args>
    void updateObservers(const std::function<void()>& f, Args&&... args) {
        (..., args.getPtr()->addObserver(f));
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
public:
    virtual ~ObserverNode() = default;

    virtual void valueChanged() {}

    void addObserver(ObserverNode* observer) { observers_.push_back(observer); }

    template <typename... Args>
    void updateObservers(Args&&... args) {
        (..., args.getPtr()->addObserver(this));
    }

    void notify() {
        for (auto* observer : observers_) {
            observer->valueChanged();
        }
    }

private:
    std::vector<ObserverNode*> observers_;
};
#endif

class ObserverGraph : public Singleton<ObserverGraph> {
public:
    void addNode(const NodePtr& node) { nodes_.insert(node); }

    void removeNode(const NodePtr& node) { nodes_.erase(node); }

private:
    std::unordered_set<NodePtr> nodes_;
};

}  // namespace pyc::reaction