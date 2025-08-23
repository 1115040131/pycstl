#include "sunny_land/game/scene/game_scene.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/sprite_component.h"
#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/engine/object/game_object.h"
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

    creatTestObject();

    Scene::init();
    spdlog::trace("GameScene 初始化完成。");
}

void GameScene::handleInput() {
    Scene::handleInput();
    testCamera();
}

void GameScene::update(std::chrono::duration<double> delta_time) { Scene::update(delta_time); }

void GameScene::render() { Scene::render(); }

void GameScene::clean() { Scene::clean(); }

void GameScene::creatTestObject() {
    spdlog::trace("在 GameScene 中创建测试对象。");
    auto test_object = std::make_unique<GameObject>("test_object");

    test_object->addComponent<TransformComponent>(glm::vec2(100));
    test_object->addComponent<SpriteComponent>(ASSET("textures/Props/big-crate.png"),
                                               context_.getResourceManager());
    // test_object->getComponent<TransformComponent>()->setScale(glm::vec2(2.0f));
    // test_object->getComponent<TransformComponent>()->setRotation(30.0f);

    addGameObject(std::move(test_object));
    spdlog::trace("test_object 创建并添加到 GameScene 中。");
}

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

}  // namespace pyc::sunny_land