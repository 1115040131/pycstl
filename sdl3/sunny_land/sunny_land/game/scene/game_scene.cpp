#include "sunny_land/game/scene/game_scene.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/component/animation_component.h"
#include "sunny_land/engine/component/tilelayer_component.h"
#include "sunny_land/engine/component/transform_component.h"
#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/input/input_manager.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/physics/physics_engine.h"
#include "sunny_land/engine/render/camera.h"
#include "sunny_land/engine/scene/level_loader.h"
#include "sunny_land/engine/utils/macro.h"
#include "sunny_land/game/component/player_component.h"

namespace pyc::sunny_land {

GameScene::GameScene(std::string_view name, Context& context, SceneManager& scene_manager)
    : Scene(name, context, scene_manager) {
    spdlog::trace("GameScene 构造完成。");
}

void GameScene::init() {
    if (is_initialized_) {
        spdlog::warn("GameScene 已经初始化过了，重复调用 init()。");
        return;
    }
    spdlog::trace("GameScene 初始化开始...");

    if (!initLevel()) {
        spdlog::error("关卡初始化失败，无法继续。");
        context_.getInputManager().setShouldQuit(true);
        return;
    }
    if (!initPlayer()) {
        spdlog::error("玩家初始化失败，无法继续。");
        context_.getInputManager().setShouldQuit(true);
        return;
    }
    if (!initEnemyAndItem()) {
        spdlog::error("敌人和道具初始化失败，无法继续。");
        context_.getInputManager().setShouldQuit(true);
        return;
    }

    Scene::init();
    spdlog::trace("GameScene 初始化完成。");
}

void GameScene::handleInput() {
    Scene::handleInput();
    testHealth();
}

void GameScene::update(std::chrono::duration<float> delta_time) { Scene::update(delta_time); }

void GameScene::render() { Scene::render(); }

void GameScene::clean() { Scene::clean(); }

bool GameScene::initLevel() {
    // 加载关卡
    LevelLoader level_loader;
    if (!level_loader.loadLevel(ASSET("maps/level1.tmj"), *this)) {
        spdlog::error("关卡加载失败");
        return false;
    }

    // 注册 main 层到物理引擎
    auto main_layer = findGameObjectByName("main");
    if (!main_layer) {
        spdlog::error("未找到\"main\"层");
        return false;
    }
    auto tile_layer = main_layer->getComponent<TileLayerComponent>();
    if (!tile_layer) {
        spdlog::error("\"main\"层没有 TileLayerComponent 组件");
    }
    context_.getPhysicsEngine().registerCollisionLayer(tile_layer);
    spdlog::info("注册\"main\"层到物理引擎");

    // 设置相机边界
    context_.getCamera().setLimitBounds(Rect{glm::vec2(0.0f), tile_layer->getWorldSize()});

    // 设置世界边界
    context_.getPhysicsEngine().setWorldBounds(Rect{glm::vec2(0.0f), tile_layer->getWorldSize()});

    spdlog::trace("关卡初始化完成。");
    return true;
}

bool GameScene::initPlayer() {
    // 获取玩家对象
    player_ = findGameObjectByName("player");
    if (!player_) {
        spdlog::error("未找到玩家对象");
        return false;
    }

    // 添加PlayerComponent到玩家对象
    auto player_component = player_->addComponent<PlayerComponent>();
    if (!player_component) {
        spdlog::error("无法添加 PlayerComponent 到玩家对象");
        return false;
    }

    // 相机跟随玩家
    auto player_transform = player_->getComponent<TransformComponent>();
    if (!player_transform) {
        spdlog::error("玩家对象没有 TransformComponent 组件, 无法设置相机目标");
        return false;
    }
    context_.getCamera().setTarget(player_transform);
    spdlog::trace("Player初始化完成。");
    return true;
}

static bool objectPlayerAnimation(std::unique_ptr<GameObject>& game_object, std::string_view animation_name) {
    if (auto ac = game_object->getComponent<AnimationComponent>()) {
        ac->playAnimation(animation_name);
        return true;
    } else {
        spdlog::error("{} 对象缺少 AnimationComponent, 无法播放动画。", game_object->getName());
        return false;
    }
}

bool GameScene::initEnemyAndItem() {
    bool success = true;
    for (auto& game_object : game_objects_) {
        if (game_object->getName() == "eagle") {
            success = success && objectPlayerAnimation(game_object, "fly");
        } else if (game_object->getName() == "frog") {
            success = success && objectPlayerAnimation(game_object, "idle");
        } else if (game_object->getName() == "opossum") {
            success = success && objectPlayerAnimation(game_object, "walk");
        } else if (game_object->getTag() == "item") {
            success = success && objectPlayerAnimation(game_object, "idle");
        }
    }
    return success;
}

void GameScene::testHealth() {
    if (context_.getInputManager().isActionPressed("attack")) {
        player_->getComponent<PlayerComponent>()->takeDamage(1);
    }
}

}  // namespace pyc::sunny_land