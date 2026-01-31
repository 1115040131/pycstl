#include "monster_war/game/scene/level_clear_scene.h"

#include <spdlog/spdlog.h>

#include "monster_war/engine/audio/audio_player.h"
#include "monster_war/engine/core/context.h"
#include "monster_war/engine/core/game_state.h"
#include "monster_war/game/data/session_data.h"
#include "monster_war/game/scene/game_scene.h"
#include "monster_war/game/scene/title_scene.h"
#include "monster_war/game/system/debug_ui_system.h"

namespace pyc::monster_war {

using namespace entt::literals;

LevelClearScene::LevelClearScene(Context& context, GameStats& game_stats,
                                 std::shared_ptr<BlueprintManager> blueprint_manager,
                                 std::shared_ptr<SessionData> session_data, std::shared_ptr<UIConfig> ui_config,
                                 std::shared_ptr<LevelConfig> level_config)
    : Scene("LevelClearScene", context),
      game_stats_(game_stats),
      blueprint_manager_(blueprint_manager),
      session_data_(session_data),
      ui_config_(ui_config),
      level_config_(level_config) {
    // 直接在构造函数中初始化DebugUI系统
    debug_ui_system_ = std::make_unique<DebugUISystem>(registry_, context);
}

LevelClearScene::~LevelClearScene() = default;

void LevelClearScene::init() {
    if (!blueprint_manager_ || !session_data_ || !ui_config_ || !level_config_) {
        spdlog::error("LevelClearScene: ui_config_, level_config_, session_data_ or blueprint_manager_ 必须有值");
        return;
    }
    context_.getGameState().setState(State::LevelClear);

    registry_.ctx().emplace<std::shared_ptr<SessionData>>(session_data_);
    registry_.ctx().emplace<std::shared_ptr<BlueprintManager>>(blueprint_manager_);
    registry_.ctx().emplace<std::shared_ptr<UIConfig>>(ui_config_);
    context_.getAudioPlayer().playMusic("win"_hs, 0);
}

void LevelClearScene::render() {
    Scene::render();
    debug_ui_system_->updateLevelClear(*this);
}

void LevelClearScene::onNextLevelClick() {
    session_data_->addOneLevel();
    session_data_->setLevelClear(true);
    requestReplaceScene(
        std::make_unique<GameScene>(context_, blueprint_manager_, session_data_, ui_config_, level_config_));
}

void LevelClearScene::onBackToTitleClick() { requestReplaceScene(std::make_unique<TitleScene>(context_)); }

void LevelClearScene::onSaveClick() { show_save_panel_ = !show_save_panel_; }

}  // namespace pyc::monster_war