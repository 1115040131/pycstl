#pragma once

#include <glm/glm.hpp>

#include "ghost_escape/affiliate/collider.h"
#include "ghost_escape/core/object_screen.h"

namespace pyc {
namespace sdl3 {

class ObjectWorld : public ObjectScreen {
public:
    virtual void init() override { type_ = Type::kWorld; }

    virtual void update(std::chrono::duration<float> delta) override;

    const glm::vec2& getPosition() const { return position_; }
    void setPosition(const glm::vec2& position);
    virtual void setRenderPosition(const glm::vec2& render_position) override;

    Collider* getCollider() const { return collider_; }
    void setCollider(Collider* collider) { collider_ = collider; }

protected:
    glm::vec2 position_{};
    Collider* collider_{};
};

}  // namespace sdl3
}  // namespace pyc
