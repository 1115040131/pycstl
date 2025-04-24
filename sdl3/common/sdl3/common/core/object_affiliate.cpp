#include "sdl3/common/core/object_affiliate.h"

#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

ObjectScreen* ObjectAffiliate::getParent() const { return static_cast<ObjectScreen*>(parent_); }

void ObjectAffiliate::setParent(Object* parent) {
    if (!dynamic_cast<ObjectScreen*>(parent)) {
        fmt::println("ObjectAffiliate::setParent: parent is not a ObjectScreen");
    }
    parent_ = parent;
}

void ObjectAffiliate::setOffsetByAnchor(Anchor anchor) {
    anchor_ = anchor;
    switch (anchor) {
        case Anchor::kTopLeft:
            offset_ = glm::vec2(0);
            break;
        case Anchor::kTopCenter:
            offset_ = glm::vec2(-size_.x / 2.0f, 0);
            break;
        case Anchor::kTopRight:
            offset_ = glm::vec2(-size_.x, 0);
            break;
        case Anchor::kCenterLeft:
            offset_ = glm::vec2(0, -size_.y / 2.0f);
            break;
        case Anchor::kCenter:
            offset_ = -size_ / 2.0f;
            break;
        case Anchor::kCenterRight:
            offset_ = glm::vec2(-size_.x, -size_.y / 2.0f);
            break;
        case Anchor::kBottomLeft:
            offset_ = glm::vec2(0, -size_.y);
            break;
        case Anchor::kBottomCenter:
            offset_ = glm::vec2(-size_.x / 2.0f, -size_.y);
            break;
        case Anchor::kBottomRight:
            offset_ = -size_;
            break;
        default:
            break;
    }
}

glm::vec2 ObjectAffiliate::getRenderPosition() const { return getParent()->getRenderPosition() + offset_; }

glm::vec2 ObjectAffiliate::getPosition() const {
    return game_.getCurrentScene()->screenToWorld(getRenderPosition());
}

}  // namespace sdl3
}  // namespace pyc
