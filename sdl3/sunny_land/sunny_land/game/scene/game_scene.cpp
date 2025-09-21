#include "sunny_land/game/scene/game_scene.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/collider_component.h"
#include "sunny_land/engine/component/physics_component.h"
#include "sunny_land/engine/component/sprite_component.h"
#include "sunny_land/engine/component/tilelayer_component.h"
#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/physics/physics_engine.h"
#include "sunny_land/engine/render/camera.h"
#include "sunny_land/engine/scene/level_loader.h"
#include "sunny_land/engine/utils/macro.h"

namespace pyc::sunny_land {

GameScene::GameScene(std::string_view name, Context& context, SceneManager& scene_manager)
    : Scene(name, context, scene_manager) {
    spdlog::trace("GameScene 构造完成。");
}

void GameScene::init() {
    // 加载关卡
    LevelLoader level_loader;
    if (!level_loader.loadLevel(ASSET("maps/level1.tmj"), *this)) {
        spdlog::error("关卡加载失败");
        return;
    }

    // 注册 main 层到物理引擎
    if (auto main_layer = findGameObjectByName("main")) {
        if (auto tile_layer = main_layer->getComponent<TileLayerComponent>()) {
            context_.getPhysicsEngine().registerCollisionLayer(tile_layer);
            spdlog::info("注册\"main\"层到物理引擎");
        }
    }

    player_ = findGameObjectByName("player");
    if (!player_) {
        spdlog::error("未找到玩家对象");
        return;
    }

    Scene::init();
    spdlog::trace("GameScene 初始化完成。");
}

void GameScene::handleInput() {
    Scene::handleInput();
    // testCamera();
    testPlayer();
}

void GameScene::update(std::chrono::duration<float> delta_time) {
    Scene::update(delta_time);
    testCollisionPairs();
}

void GameScene::render() { Scene::render(); }

void GameScene::clean() { Scene::clean(); }

void GameScene::testCamera() {
    auto& camera_ = context_.getCamera();
    auto& input_manager_ = context_.getInputManager();
    if (input_manager_.isActionDown("move_up")) {
        camera_.move(glm::vec2(0, -1));
    }
    if (input_manager_.isActionDown("move_down")) {
        camera_.move(glm::vec2(0, 1));
    }
    if (input_manager_.isActionDown("move_left")) {
        camera_.move(glm::vec2(-1, 0));
    }
    if (input_manager_.isActionDown("move_right")) {
        camera_.move(glm::vec2(1, 0));
    }
}

void GameScene::testPlayer() {
    if (!player_) {
        return;
    }

    auto physics = player_->getComponent<PhysicsComponent>();
    if (!physics) {
        return;
    }

    const auto& input_manager_ = context_.getInputManager();
    const auto& velocity = physics->getVelocity();
    if (input_manager_.isActionDown("move_left")) {
        physics->setVelocity({-100.0f, velocity.y});
    } else {
        physics->setVelocity({velocity.x * 0.9f, velocity.y});
    }
    if (input_manager_.isActionDown("move_right")) {
        physics->setVelocity({100.0f, velocity.y});
    } else {
        physics->setVelocity({velocity.x * 0.9f, velocity.y});
    }
    if (input_manager_.isActionDown("jump")) {
        physics->setVelocity({velocity.x, -400.0f});
    }
}

void GameScene::testCollisionPairs() {
    auto& physics_engine = context_.getPhysicsEngine();
    for (const auto& pair : physics_engine.getCollisionPairs()) {
        spdlog::info("检测到碰撞: {} <-> {}", pair.first->getName(), pair.second->getName());
    }
}

}  // namespace pyc::sunny_land