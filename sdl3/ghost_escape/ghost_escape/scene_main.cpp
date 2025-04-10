#include "ghost_escape/scene_main.h"

#include "ghost_escape/player.h"
#include "ghost_escape/spawner.h"
#include "sdl3/common/screen/ui_mouse.h"

namespace pyc {
namespace sdl3 {

void SceneMain::init() {
    SDL_HideCursor();

    world_size_ = game_.getScreenSize() * 3.0F;
    camera_position_ = world_size_ / 2.F - game_.getScreenSize() / 2.F;

    auto player = std::make_unique<Player>();
    player->init();
    player->setPosition(world_size_ / 2.F);
    player_ = static_cast<Player*>(addChild(std::move(player)));

    auto spawner = std::make_unique<Spawner>();
    spawner->init();
    spawner->setTarget(player_);
    spawner_ = static_cast<Spawner*>(addChild(std::move(spawner)));

    ui_mouse_ = UIMouse::CreateAndSet(this, ASSET("UI/29.png"), ASSET("UI/30.png"), 2.0F);
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
