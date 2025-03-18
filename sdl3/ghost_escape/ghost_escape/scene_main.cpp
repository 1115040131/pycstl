#include "ghost_escape/scene_main.h"

namespace pyc {
namespace sdl3 {

void SceneMain::init() {
    world_size_ = game_.screenSize() * 3.0F;
    camera_position_ = glm::vec2(-100.F);
}

void SceneMain::clean() {}

void SceneMain::handleEvents(SDL_Event&) {}

void SceneMain::update(std::chrono::duration<float> delta) { camera_position_ += glm::vec2(80.F) * delta.count(); }

void SceneMain::render() { renderBackground(); }

void SceneMain::renderBackground() {
    auto start = -camera_position_;
    auto end = world_size_ - camera_position_;
    game_.drawGrid(start, end, 80.0F, 80.0F, {0.5, 0.5, 0.5, 1});
    game_.drawBoundary(start, end, 5.F, {1, 1, 1, 1});
}

}  // namespace sdl3
}  // namespace pyc
