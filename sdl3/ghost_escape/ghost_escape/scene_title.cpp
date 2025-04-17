#include "ghost_escape/scene_title.h"

#include <fmt/format.h>

#include "sdl3/common/screen/hud_text.h"

namespace pyc {
namespace sdl3 {

void SceneTitle::init() {
#ifdef DEBUG_MODE
    name_ = "SceneTitle";
#endif
    HUDText::CreateAndSet(this, "幽灵逃生", game_.getScreenSize() / 2.0f - glm::vec2(0, 100),
                          glm::vec2(game_.getScreenSize().x / 2.0f, game_.getScreenSize().y / 3.0f),
                          ASSET("font/VonwaonBitmap-16px.ttf"), 64, ASSET("UI/Textfield_01.png"));
    HUDText::CreateAndSet(this, fmt::format("最高分: {}", game_.getHighScore()),
                          game_.getScreenSize() / 2.0f + glm::vec2(0, 100), glm::vec2(200, 50),
                          ASSET("font/VonwaonBitmap-16px.ttf"), 32, ASSET("UI/Textfield_01.png"));
}

void SceneTitle::update(std::chrono::duration<float> delta) {
    Scene::update(delta);
    updateColor(delta);
}

void SceneTitle::render() {
    renderBackground();
    Scene::render();
}

void SceneTitle::updateColor(std::chrono::duration<float> delta) {
    timer_ += delta;
    boundary_color_.r = 0.5f + 0.5 * std::sin(timer_.count() * 0.9f);
    boundary_color_.g = 0.5f + 0.5 * std::sin(timer_.count() * 0.8f);
    boundary_color_.b = 0.5f + 0.5 * std::sin(timer_.count() * 0.7f);
}

void SceneTitle::renderBackground() const {
    game_.drawBoundary(glm::vec2(30), game_.getScreenSize() - glm::vec2(30), 10.0f, boundary_color_);
}

}  // namespace sdl3
}  // namespace pyc