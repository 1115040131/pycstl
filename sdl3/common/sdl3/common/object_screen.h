#pragma once

#include <glm/glm.hpp>

#include "sdl3/common/object.h"

namespace pyc {
namespace sdl3 {

class ObjectScreen : public Object {
public:
    virtual void init() override { type_ = Type::kScreen; }

    const glm::vec2& getRenderPosition() const { return render_position_; }
    virtual void setRenderPosition(const glm::vec2& render_position) { render_position_ = render_position; }

protected:
    glm::vec2 render_position_{};
};

}  // namespace sdl3
}  // namespace pyc
