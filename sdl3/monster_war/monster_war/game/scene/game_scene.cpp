#include "monster_war/game/scene/game_scene.h"

#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/core/context.h"
#include "monster_war/engine/input/input_manager.h"
#include "monster_war/engine/utils/events.h"

namespace pyc::monster_war {

GameScene::GameScene(Context& context, SceneManager& scene_manager) : Scene("GameScene", context, scene_manager) {
    spdlog::trace("GameScene 构造完成。");
}

void GameScene::init() {
    // 注册输入回调
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("attack").connect<&GameScene::onAttack>(this);
    input_manager.onAction("jump", ActionState::RELEASED).connect<&GameScene::onJump>(this);
}

void GameScene::clean() {
    // 断开输入回调
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("attack").disconnect<&GameScene::onAttack>(this);
    input_manager.onAction("jump", ActionState::RELEASED).disconnect<&GameScene::onJump>(this);
}

void GameScene::onAttack() {
    spdlog::info("onAttack");
    // 按攻击键即发送“退出游戏”信号
    context_.getDispatcher().enqueue<QuitEvent>();
}

void GameScene::onJump() { spdlog::info("onJump"); }

}  // namespace pyc::monster_war