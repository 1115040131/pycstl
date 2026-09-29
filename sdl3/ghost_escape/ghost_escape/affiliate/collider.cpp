module;

#include <memory>
#include <utility>

#include <glm/glm.hpp>

module ghost_escape.core;

namespace pyc::sdl3 {

Collider* Collider::CreateAndSet(ObjectWorld* parent, glm::vec2 size, Type type, Anchor anchor) {
    auto collider = std::make_unique<Collider>();
    collider->init();
    SetDebugName(collider.get());
    collider->setSize(size);
    collider->type_ = type;
    collider->setOffsetByAnchor(anchor);
    return static_cast<Collider*>(parent->addChild(std::move(collider)));
}

void Collider::render() {
#ifdef DEBUG_MODE
    game_.renderFillCircle(Asset("UI/circle.png"), getRenderPosition(), size_, 0.5F);
#endif
}

bool Collider::isColliding(const Collider& other) const {
    if (type_ == Type::kCircle && other.type_ == Type::kCircle) {
        return glm::distance(getPosition() + glm::vec2(size_.x / 2.0f),
                             other.getPosition() + glm::vec2(other.size_.x / 2.0f)) <
               (size_.x + other.size_.x) / 2.0f;
    }
    return false;
}

}  // namespace pyc::sdl3
