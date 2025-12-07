#include "monster_war/game/scene/game_scene.h"

#include <spdlog/spdlog.h>

namespace pyc::monster_war {

GameScene::GameScene(Context& context, SceneManager& scene_manager) : Scene("GameScene", context, scene_manager) {
    spdlog::trace("GameScene 构造完成。");
}

}  // namespace pyc::monster_war