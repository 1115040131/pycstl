#include "monster_war/game/scene/end_scene.h"

#include <spdlog/spdlog.h>

#include "monster_war/engine/audio/audio_player.h"
#include "monster_war/engine/core/context.h"
#include "monster_war/engine/core/game_state.h"
#include "monster_war/game/scene/title_scene.h"
#include "monster_war/game/system/debug_ui_system.h"

namespace pyc::monster_war {

using namespace entt::literals;

EndScene::EndScene(Context& context, bool is_win) : Scene("EndScene", context), is_win_(is_win) {
    // 直接在构造函数中初始化DebugUI系统
    debug_ui_system_ = std::make_unique<DebugUISystem>(registry_, context);
}

EndScene::~EndScene() = default;

void EndScene::init() {
    context_.getGameState().setState(State::GameOver);
    if (is_win_) {
        context_.getAudioPlayer().playMusic("win"_hs, 0);
    } else {
        context_.getAudioPlayer().playMusic("lose"_hs, 0);
    }
}

void EndScene::render() {
    Scene::render();
    debug_ui_system_->updateEnd(*this);
}

void EndScene::onBackToTitleClick() { requestReplaceScene(std::make_unique<TitleScene>(context_)); }

void EndScene::onQuitClick() { quit(); }

}  // namespace pyc::monster_war