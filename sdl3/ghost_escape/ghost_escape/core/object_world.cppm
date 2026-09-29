module;

#include <chrono>

#include <glm/glm.hpp>

export module ghost_escape.core:world;

import :collider;
export import :screen;

export namespace pyc::sdl3 {

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

}  // namespace pyc::sdl3
