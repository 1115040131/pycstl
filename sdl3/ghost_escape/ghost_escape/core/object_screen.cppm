module;

#include <glm/glm.hpp>

export module ghost_escape.core:screen;

export import :object;

export namespace pyc::sdl3 {

class ObjectScreen : public Object {
public:
    virtual void init() override { type_ = Type::kScreen; }

    const glm::vec2& getRenderPosition() const { return render_position_; }
    virtual void setRenderPosition(const glm::vec2& render_position) { render_position_ = render_position; }

protected:
    glm::vec2 render_position_{};
};

}  // namespace pyc::sdl3
