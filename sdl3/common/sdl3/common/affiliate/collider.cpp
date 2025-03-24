#include "sdl3/common/affiliate/collider.h"

#include "sdl3/common/core/object_world.h"

namespace pyc {
namespace sdl3 {

Collider* Collider::CreateAndSet(ObjectWorld* parent, glm::vec2 size, Type type) {
    auto collider = std::make_unique<Collider>();
    collider->init();
    collider->setSize(size);
    collider->type_ = type;
    return static_cast<Collider*>(parent->addChild(std::move(collider)));
}

void Collider::render() {
#ifdef DEBUG_MODE
    game_.renderFillCircle(getPosition(), size_, 0.5F);
#endif
}

bool Collider::isColliding(const Collider& other) const {
    if (type_ == Type::kCircle && other.type_ == Type::kCircle) {
        return glm::distance(getPosition(), other.getPosition()) < (size_.x + other.size_.x) / 2.F;
    }
    return false;
}

}  // namespace sdl3
}  // namespace pyc
