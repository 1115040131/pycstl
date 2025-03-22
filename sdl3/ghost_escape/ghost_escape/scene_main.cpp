#include "ghost_escape/scene_main.h"

#include "ghost_escape/enemy.h"

namespace pyc {
namespace sdl3 {

void SceneMain::init() {
    world_size_ = game_.getScreenSize() * 3.0F;
    camera_position_ = world_size_ / 2.F - game_.getScreenSize() / 2.F;

    auto player = std::make_unique<Player>();
    player_ = player.get();
    player_->init();
    player_->setPosition(world_size_ / 2.F);
    addChild(std::move(player));

    auto enemy = std::make_unique<Enemy>();
    enemy->init();
    enemy->setPosition(world_size_ / 2.F + glm::vec2(200));
    enemy->setTarget(player_);
    addChild(std::move(enemy));
}

void SceneMain::clean() { Scene::clean(); }

void SceneMain::handleEvents(const SDL_Event& event) { Scene::handleEvents(event); }

void SceneMain::update(std::chrono::duration<float> delta) {
    Scene::update(delta);
    fmt::println("children_world: {}, children_scrren: {}, children: {}", children_world_.size(),
                 children_screen_.size(), children_.size());
}

void SceneMain::render() {
    renderBackground();
    Scene::render();
}

void SceneMain::renderBackground() const {
    auto start = -camera_position_;
    auto end = world_size_ - camera_position_;
    game_.drawGrid(start, end, 80.0F, 80.0F, {0.5, 0.5, 0.5, 1});
    game_.drawBoundary(start, end, 5.F, {1, 1, 1, 1});
}

}  // namespace sdl3
}  // namespace pyc
