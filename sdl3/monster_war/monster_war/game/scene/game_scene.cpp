#include "monster_war/game/scene/game_scene.h"

#include <entt/core/hashed_string.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/audio/audio_player.h"
#include "monster_war/engine/core/context.h"
#include "monster_war/engine/resource/resource_manager.h"
#include "monster_war/engine/ui/ui_image.h"
#include "monster_war/engine/ui/ui_label.h"
#include "monster_war/engine/ui/ui_manager.h"
#include "monster_war/engine/utils/events.h"

namespace pyc::monster_war {

using namespace entt::literals;

GameScene::GameScene(Context& context) : Scene("GameScene", context) { spdlog::trace("GameScene 构造完成。"); }

void GameScene::init() {
    // 测试资源管理器
    testResourceManager();

    Scene::init();
}

void GameScene::clean() { Scene::clean(); }

void GameScene::testResourceManager() {
    // 载入资源
    context_.getResourceManager().loadTexture("assets/textures/Buildings/Castle.png"_hs);
    // 播放音乐
    context_.getAudioPlayer().playMusic("battle_bgm"_hs);

    // 测试UI元素（使用载入的资源）
    ui_manager_->addElement(std::make_unique<UIImage>("assets/textures/Buildings/Castle.png"_hs));
    ui_manager_->addElement(std::make_unique<UILabel>(context_.getTextRenderer(), "Hello, World!",
                                                      "assets/fonts/VonwaonBitmap-16px.ttf"));
}

}  // namespace pyc::monster_war