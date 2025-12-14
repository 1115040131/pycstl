#include "monster_war/game/scene/game_scene.h"

#include <entt/core/hashed_string.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/core/context.h"
#include "monster_war/engine/loader/level_loader.h"
#include "monster_war/engine/system/animation_system.h"
#include "monster_war/engine/system/movement_system.h"
#include "monster_war/engine/system/render_system.h"
#include "monster_war/engine/system/ysort_system.h"
#include "monster_war/engine/ui/ui_manager.h"
#include "monster_war/engine/utils/events.h"

namespace pyc::monster_war {

using namespace entt::literals;

GameScene::GameScene(Context& context) : Scene("GameScene", context) {
    // 初始化系统
    render_system_ = std::make_unique<RenderSystem>();
    movement_system_ = std::make_unique<MovementSystem>();
    animation_system_ = std::make_unique<AnimationSystem>();
    ysort_system_ = std::make_unique<YSortSystem>();

    spdlog::trace("GameScene 构造完成。");
}

GameScene::~GameScene() = default;

void GameScene::init() {
    if (!loadLevel()) {
        spdlog::error("加载关卡失败");
        return;
    }

    Scene::init();
}

void GameScene::update(std::chrono::duration<float> delta_time) {
    movement_system_->update(registry_, delta_time);
    animation_system_->update(registry_, delta_time);
    ysort_system_->update(registry_);  // 调用顺序要在MovementSystem之后

    Scene::update(delta_time);
}

void GameScene::render() {
    render_system_->update(registry_, context_.getRenderer(), context_.getCamera());

    Scene::render();
}

void GameScene::clean() { Scene::clean(); }

bool GameScene::loadLevel() {
    LevelLoader level_loader;
    // 不调用setEntityBuilder，则使用默认的BasicEntityBuilder
    if (!level_loader.loadLevel("assets/maps/level1.tmj", this)) {
        spdlog::error("加载关卡失败");
        return false;
    }
    return true;
}

}  // namespace pyc::monster_war