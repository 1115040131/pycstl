#include "monster_war/game/scene/title_scene.h"

#include <spdlog/spdlog.h>

#include "monster_war/engine/audio/audio_player.h"
#include "monster_war/engine/core/context.h"
#include "monster_war/engine/core/game_state.h"
#include "monster_war/engine/core/time.h"
#include "monster_war/engine/loader/level_loader.h"
#include "monster_war/engine/system/animation_system.h"
#include "monster_war/engine/system/movement_system.h"
#include "monster_war/engine/system/render_system.h"
#include "monster_war/engine/system/ysort_system.h"
#include "monster_war/engine/ui/ui_manager.h"
#include "monster_war/game/data/level_config.h"
#include "monster_war/game/data/session_data.h"
#include "monster_war/game/data/ui_config.h"
#include "monster_war/game/factory/blueprint_manager.h"
#include "monster_war/game/scene/game_scene.h"
#include "monster_war/game/system/debug_ui_system.h"

namespace pyc::monster_war {

using namespace entt::literals;

TitleScene::TitleScene(Context& context, std::shared_ptr<BlueprintManager> blueprint_manager,
                       std::shared_ptr<SessionData> session_data, std::shared_ptr<UIConfig> ui_config,
                       std::shared_ptr<LevelConfig> level_config)
    : Scene("TitleScene", context),
      blueprint_manager_(blueprint_manager),
      session_data_(session_data),
      ui_config_(ui_config),
      level_config_(level_config) {}

TitleScene::~TitleScene() = default;

void TitleScene::init() {
    if (!initSessionData()) {
        spdlog::error("初始化session_data_失败");
        return;
    }
    if (!initLevelConfig()) {
        spdlog::error("初始化关卡配置失败");
        return;
    }
    if (!initBlueprintManager()) {
        spdlog::error("初始化蓝图管理器失败");
        return;
    }
    if (!initUIConfig()) {
        spdlog::error("初始化UI配置失败");
        return;
    }
    if (!loadTitleLevel()) {
        spdlog::error("加载关卡失败");
        return;
    }
    if (!initSystems()) {
        spdlog::error("初始化系统失败");
        return;
    }
    if (!initRegistryContext()) {
        spdlog::error("初始化注册表上下文失败");
        return;
    }
    if (!initUI()) {
        spdlog::error("初始化UI失败");
        return;
    }

    context_.getGameState().setState(State::Title);
    context_.getTime().setTimeScale(1.0f);                // 重置游戏速度
    context_.getAudioPlayer().playMusic("title_bgm"_hs);  // 设置标题场景背景音乐

    Scene::init();
}

void TitleScene::update(std::chrono::duration<float> delta_time) {
    Scene::update(delta_time);
    animation_system_->update(delta_time);
    movement_system_->update(registry_, delta_time);
    ysort_system_->update(registry_);
}

void TitleScene::render() {
    auto& renderer = context_.getRenderer();
    auto& camera = context_.getCamera();

    render_system_->update(registry_, renderer, camera);

    Scene::render();
    debug_ui_system_->updateTitle(*this);
}

bool TitleScene::initSessionData() {
    if (!session_data_) {
        session_data_ = std::make_shared<SessionData>();
        if (!session_data_->loadDefaultData()) {
            spdlog::error("初始化session_data_失败");
            return false;
        }
    }
    return true;
}

bool TitleScene::initLevelConfig() {
    if (!level_config_) {
        level_config_ = std::make_shared<LevelConfig>();
        if (!level_config_->loadFromFile("assets/data/level_config.json")) {
            spdlog::error("加载关卡配置失败");
            return false;
        }
    }
    return true;
}

bool TitleScene::initBlueprintManager() {  // 如果蓝图管理器为空，则创建一个（将来可能由构造函数传入）
    if (!blueprint_manager_) {
        blueprint_manager_ = std::make_shared<BlueprintManager>(context_.getResourceManager());
        if (!blueprint_manager_->loadEnemyClassBlueprints("assets/data/enemy_data.json") ||
            !blueprint_manager_->loadPlayerClassBlueprints("assets/data/player_data.json") ||
            !blueprint_manager_->loadProjectileBlueprints("assets/data/projectile_data.json") ||
            !blueprint_manager_->loadEffectBlueprints("assets/data/effect_data.json") ||
            !blueprint_manager_->loadSkillBlueprints("assets/data/skill_data.json")) {
            spdlog::error("加载蓝图失败");
            return false;
        }
    }
    spdlog::info("entity_factory_ 加载完成");
    return true;
}

bool TitleScene::initUIConfig() {
    if (!ui_config_) {
        ui_config_ = std::make_shared<UIConfig>();
        if (!ui_config_->loadFromFile("assets/data/ui_config.json")) {
            spdlog::error("加载UI配置失败");
            return false;
        }
    }
    return true;
}

bool TitleScene::loadTitleLevel() {
    LevelLoader level_loader;
    if (!level_loader.loadLevel("assets/maps/title.tmj", this)) {
        spdlog::error("加载关卡失败");
        return false;
    }
    return true;
}

bool TitleScene::initSystems() {
    // 初始化系统
    auto& dispatcher = context_.getDispatcher();
    debug_ui_system_ = std::make_unique<DebugUISystem>(registry_, context_);
    render_system_ = std::make_unique<RenderSystem>();
    ysort_system_ = std::make_unique<YSortSystem>();
    animation_system_ = std::make_unique<AnimationSystem>(registry_, dispatcher);
    movement_system_ = std::make_unique<MovementSystem>();
    return true;
}

bool TitleScene::initRegistryContext() {  // 让注册表存储一些数据类型实例作为上下文，方便使用
    registry_.ctx().emplace<std::shared_ptr<SessionData>>(session_data_);
    registry_.ctx().emplace<std::shared_ptr<BlueprintManager>>(blueprint_manager_);
    registry_.ctx().emplace<std::shared_ptr<UIConfig>>(ui_config_);
    return true;
}

bool TitleScene::initUI() {
    auto window_size = context_.getGameState().getLogicalSize();
    if (!ui_manager_->init(window_size)) {
        return false;
    }

    // 设置背景音乐
    // context_.getAudioPlayer().playMusic("title_bgm"_hs);

    /* 先用ImGui实现UI，未来再使用游戏内UI */
    return true;
}

void TitleScene::onStartGameClick() {
    // 如果数据是读档载入的，有可能已经通关，此时需要进入下一关
    if (session_data_->isLevelClear()) {
        session_data_->setLevelClear(false);
        session_data_->addOneLevel();
    }
    requestReplaceScene(
        std::make_unique<GameScene>(context_, blueprint_manager_, session_data_, ui_config_, level_config_));
}

void TitleScene::onConfirmRoleClick() { show_unit_info_ = !show_unit_info_; }

void TitleScene::onLoadGameClick() { show_load_panel_ = !show_load_panel_; }

void TitleScene::onQuitClick() { quit(); }

}  // namespace pyc::monster_war