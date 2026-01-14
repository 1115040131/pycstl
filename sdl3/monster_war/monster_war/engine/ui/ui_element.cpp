#include "monster_war/engine/ui/ui_element.h"

#include <algorithm>

namespace pyc::monster_war {

UIElement::UIElement(glm::vec2 position, glm::vec2 size)
    : position_(std::move(position)), size_(std::move(size)) {}

void UIElement::update(std::chrono::duration<float> delta_time, Context& context) {
    // 如果元素不可见，直接返回
    if (!visible_) {
        return;
    }

    auto partition_it =
        std::partition(children_.begin(), children_.end(), [&](const std::unique_ptr<UIElement>& child) {
            if (child && !child->need_remove_) {
                child->update(delta_time, context);
                return true;
            } else {
                return false;
            }
        });
    children_.erase(partition_it, children_.end());
}

void UIElement::render(Context& context) {
    if (!visible_) {
        return;
    }
    for (const auto& child : children_) {
        child->render(context);
    }
}

void UIElement::addChild(std::unique_ptr<UIElement> child, int order_index) {
    if (child) {
        child->setParent(this);
        if (order_index >= 0) {
            child->setOrderIndex(order_index);
        }
        children_.push_back(std::move(child));
    }
}

std::unique_ptr<UIElement> UIElement::removeChild(UIElement* child_ptr) {
    auto it = std::ranges::find_if(
        children_, [child_ptr](const std::unique_ptr<UIElement>& child) { return child.get() == child_ptr; });
    if (it != children_.end()) {
        std::unique_ptr<UIElement> removed_child = std::move(*it);
        children_.erase(it);
        removed_child->setParent(nullptr);
        return removed_child;
    }
    return nullptr;
}

std::unique_ptr<UIElement> UIElement::removeChildById(entt::id_type id) {
    auto it = std::ranges::find_if(children_,
                                   [id](const std::unique_ptr<UIElement>& child) { return child->getId() == id; });

    if (it != children_.end()) {
        std::unique_ptr<UIElement> removed_child = std::move(*it);
        children_.erase(it);
        removed_child->setParent(nullptr);
        return removed_child;
    }
    return nullptr;
}

void UIElement::removeAllChildren() { children_.clear(); }

UIElement* UIElement::getChildById(entt::id_type id) const {
    auto it = std::ranges::find_if(children_,
                                   [id](const std::unique_ptr<UIElement>& child) { return child->getId() == id; });
    if (it != children_.end()) {
        return it->get();
    }
    return nullptr;
}

void UIElement::sortChildrenByOrderIndex() {
    // 使用 stable_sort 避免破坏原来相等元素的顺序
    std::ranges::stable_sort(children_,
                             [](const std::unique_ptr<UIElement>& lhs, const std::unique_ptr<UIElement>& rhs) {
                                 return lhs->getOrderIndex() < rhs->getOrderIndex();
                             });
}

Rect UIElement::getBounds() const {
    auto screen_pos = getScreenPosition();
    return Rect{screen_pos, size_};
}

glm::vec2 UIElement::getScreenPosition() const {
    if (parent_) {
        return parent_->getScreenPosition() + position_;
    } else {
        return position_;
    }
}

bool UIElement::isPointInside(const glm::vec2& point) const {
    auto bounds = getBounds();
    return point.x >= bounds.position.x && point.x < (bounds.position.x + bounds.size.x) &&
           point.y >= bounds.position.y && point.y < (bounds.position.y + bounds.size.y);
}

}  // namespace pyc::monster_war