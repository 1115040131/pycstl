#include "sunny_land/engine/ui/ui_element.h"

namespace pyc::sunny_land {

UIElement::UIElement(glm::vec2 position, glm::vec2 size)
    : position_(std::move(position)), size_(std::move(size)) {}

bool UIElement::handleInput(Context& context) {
    if (!visible_) {
        return false;
    }

    bool event_handled = false;
    auto partition_it =
        std::partition(children_.begin(), children_.end(), [&](const std::unique_ptr<UIElement>& child) {
            if (child && !child->need_remove_) {
                if (!event_handled) {
                    event_handled = child->handleInput(context);
                }
                return true;
            } else {
                return false;
            }
        });
    children_.erase(partition_it, children_.end());
    return event_handled;
}

void UIElement::update(std::chrono::duration<float> delta_time, Context& context) {
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

void UIElement::addChild(std::unique_ptr<UIElement> child) {
    if (child) {
        child->setParent(this);
        children_.push_back(std::move(child));
    }
}

std::unique_ptr<UIElement> UIElement::removeChild(UIElement* child_ptr) {
    auto it =
        std::find_if(children_.begin(), children_.end(),
                     [child_ptr](const std::unique_ptr<UIElement>& child) { return child.get() == child_ptr; });
    if (it != children_.end()) {
        std::unique_ptr<UIElement> removed_child = std::move(*it);
        children_.erase(it);
        removed_child->setParent(nullptr);
        return removed_child;
    }
    return nullptr;
}

void UIElement::removeAllChildren() { children_.clear(); }

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

}  // namespace pyc::sunny_land