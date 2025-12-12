#include "monster_war/game/scene/game_scene.h"

#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/core/context.h"
#include "monster_war/engine/input/input_manager.h"
#include "monster_war/engine/utils/events.h"

namespace pyc::monster_war {

GameScene::GameScene(Context& context) : Scene("GameScene", context) { spdlog::trace("GameScene 构造完成。"); }

void GameScene::init() {
    // 测试场景编号, 每创建一个场景, 编号加1
    static int count = 0;
    scene_num_ = count++;
    spdlog::info("场景编号: {}", scene_num_);

    // 注册输入回调事件
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("jump").connect<&GameScene::onReplace>(this);     // J 键
    input_manager.onAction("mouse_left").connect<&GameScene::onPush>(this);  // 鼠标左键
    input_manager.onAction("mouse_right").connect<&GameScene::onPop>(this);  // 鼠标右键
    input_manager.onAction("pause").connect<&GameScene::onQuit>(this);       // P 键

    Scene::init();
}

void GameScene::clean() {
    // 断开输入回调事件 (谁连接，谁负责断开)
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("jump").disconnect<&GameScene::onReplace>(this);
    input_manager.onAction("mouse_left").disconnect<&GameScene::onPush>(this);
    input_manager.onAction("mouse_right").disconnect<&GameScene::onPop>(this);
    input_manager.onAction("pause").disconnect<&GameScene::onQuit>(this);

    Scene::clean();
}

void GameScene::onReplace() {
    spdlog::info("onReplace, 切换场景");
    requestReplaceScene(std::make_unique<GameScene>(context_));
}

void GameScene::onPush() {
    spdlog::info("onPush, 压入场景");
    requestPushScene(std::make_unique<GameScene>(context_));
}

void GameScene::onPop() {
    spdlog::info("onPop, 弹出编号为 {} 的场景", scene_num_);
    requestPopScene();
}

void GameScene::onQuit() {
    spdlog::info("onQuit, 退出游戏");
    quit();
}

}  // namespace pyc::monster_war