#include "sdl3/common/core/object_affiliate.h"

#include "sdl3/common/core/object_world.h"

namespace pyc {
namespace sdl3 {

void ObjectAffiliate::setOffsetByAnchor(Anchor anchor) {
    anchor_ = anchor;
    switch (anchor) {
        case Anchor::kTopLeft:
            offset_ = size_ / 2.F;
            break;
        case Anchor::kTopCenter:
            offset_ = glm::vec2(0, size_.y / 2.F);
            break;
        case Anchor::kTopRight:
            offset_ = glm::vec2(-size_.x / 2.F, size_.y / 2.F);
            break;
        case Anchor::kCenterLeft:
            offset_ = glm::vec2(size_.x / 2.F, 0);
            break;
        case Anchor::kCenter:
            offset_ = glm::vec2(0);
            break;
        case Anchor::kCenterRight:
            offset_ = glm::vec2(-size_.x / 2.F, 0);
            break;
        case Anchor::kBottomLeft:
            offset_ = glm::vec2(size_.x / 2.F, -size_.y / 2.F);
            break;
        case Anchor::kBottomCenter:
            offset_ = glm::vec2(0, -size_.y / 2.F);
            break;
        case Anchor::kBottomRight:
            offset_ = -size_ / 2.F;
            break;
        default:
            break;
    }
}

glm::vec2 ObjectAffiliate::getRenderPosition() const {
    return (parent_ && (parent_->getType() == Object::Type::kScreen || parent_->getType() == Object::Type::kWorld))
               ? static_cast<ObjectScreen*>(parent_)->getRenderPosition() + offset_
               : offset_;
}

glm::vec2 ObjectAffiliate::getPosition() const {
    return (parent_ && parent_->getType() == Object::Type::kWorld)
               ? static_cast<ObjectWorld*>(parent_)->getRenderPosition() + offset_
               : offset_;
}

}  // namespace sdl3
}  // namespace pyc
