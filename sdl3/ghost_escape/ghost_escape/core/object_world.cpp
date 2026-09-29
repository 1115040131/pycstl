module;

#include <chrono>

#include <glm/glm.hpp>

module ghost_escape.core;

namespace pyc::sdl3 {

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

}  // namespace pyc::sdl3
