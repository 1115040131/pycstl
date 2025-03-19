#pragma once

#include <glm/glm.hpp>

#include "sdl3/common/object_screen.h"

namespace pyc {
namespace sdl3 {

class ObjectWorld : public ObjectScreen {
public:
    virtual void update(std::chrono::duration<float> delta) override;

    const glm::vec2& getPosition() const { return position_; }
    void setPosition(const glm::vec2& position);
    virtual void setRenderPosition(const glm::vec2& render_position) override;

protected:
    glm::vec2 position_{};
};

}  // namespace sdl3
}  // namespace pyc
