#include "sdl3/common/core/object_affiliate.h"

#include "sdl3/common/core/object_world.h"

namespace pyc {
namespace sdl3 {

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
