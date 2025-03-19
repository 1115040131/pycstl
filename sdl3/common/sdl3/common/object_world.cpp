#include "sdl3/common/object_world.h"

#include "sdl3/common/scene.h"

namespace pyc {
namespace sdl3 {

void ObjectWorld::update(std::chrono::duration<float> delta) {
    ObjectScreen::update(delta);
    render_position_ = game_.getCurrentScene()->worldToScreen(position_);
}

void ObjectWorld::setPosition(const glm::vec2& position) {
    position_ = position;
    render_position_ = game_.getCurrentScene()->worldToScreen(position_);
}

void ObjectWorld::setRenderPosition(const glm::vec2& render_position) {
    render_position_ = render_position;
    position_ = game_.getCurrentScene()->screenToWorld(render_position_);
}

}  // namespace sdl3
}  // namespace pyc
