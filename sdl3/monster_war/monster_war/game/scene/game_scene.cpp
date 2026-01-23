#include "monster_war/game/scene/game_scene.h"

#include <entt/core/hashed_string.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/core/context.h"
#include "monster_war/engine/input/input_manager.h"
#include "monster_war/engine/loader/level_loader.h"
#include "monster_war/engine/system/animation_system.h"
#include "monster_war/engine/system/audio_system.h"
#include "monster_war/engine/system/movement_system.h"
#include "monster_war/engine/system/render_system.h"
#include "monster_war/engine/system/ysort_system.h"
#include "monster_war/game/component/player_component.h"
#include "monster_war/game/data/level_config.h"
#include "monster_war/game/data/session_data.h"
#include "monster_war/game/data/ui_config.h"
#include "monster_war/game/def/events.h"
#include "monster_war/game/factory/blueprint_manager.h"
#include "monster_war/game/factory/entity_factory.h"
#include "monster_war/game/loader/entity_builder_mw.h"
#include "monster_war/game/spawner/enemy_spawner.h"
#include "monster_war/game/system/animation_event_system.h"
#include "monster_war/game/system/animation_state_system.h"
#include "monster_war/game/system/attack_starter_system.h"
#include "monster_war/game/system/block_system.h"
#include "monster_war/game/system/combat_resolve_system.h"
#include "monster_war/game/system/debug_ui_system.h"
#include "monster_war/game/system/effect_system.h"
#include "monster_war/game/system/followpath_system.h"
#include "monster_war/game/system/game_rule_system.h"
#include "monster_war/game/system/health_bar_system.h"
#include "monster_war/game/system/orientation_system.h"
#include "monster_war/game/system/place_unit_system.h"
#include "monster_war/game/system/projectile_system.h"
#include "monster_war/game/system/remove_dead_system.h"
#include "monster_war/game/system/render_range_system.h"
#include "monster_war/game/system/selection_system.h"
#include "monster_war/game/system/set_target_system.h"
#include "monster_war/game/system/skill_system.h"
#include "monster_war/game/system/timer_system.h"
#include "monster_war/game/ui/units_portrait_ui.h"

namespace pyc::monster_war {

using namespace entt::literals;

GameScene::GameScene(Context& context) : Scene("GameScene", context) { spdlog::trace("GameScene 构造完成。"); }

GameScene::~GameScene() = default;

void GameScene::init() {
    if (!initSessionData()) {
        spdlog::error("初始化session_data_失败");
        return;
    }
    if (!initLevelConfig()) {
        spdlog::error("初始化关卡配置失败");
        return;
    }
    if (!initUIConfig()) {
        spdlog::error("初始化UI配置失败");
        return;
    }
    if (!loadLevel()) {
        spdlog::error("加载关卡失败");
        return;
    }
    if (!initEventConnections()) {
        spdlog::error("初始化事件连接失败");
        return;
    }
    if (!initInputConnections()) {
        spdlog::error("初始化输入连接失败");
        return;
    }
    if (!initEntityFactory()) {
        spdlog::error("初始化实体工厂失败");
        return;
    }
    if (!initRegistryContext()) {
        spdlog::error("初始化注册表上下文失败");
        return;
    }
    if (!initSystems()) {
        spdlog::error("初始化系统失败");
        return;
    }
    if (!initEnemySpawner()) {
        spdlog::error("初始化敌人生成器失败");
        return;
    }
    if (!initUnitsPortraitUI()) {
        spdlog::error("初始化单位肖像UI失败");
        return;
    }

    Scene::init();
}

void GameScene::update(std::chrono::duration<float> delta_time) {
    auto& dispatcher = context_.getDispatcher();

    // 每一帧最先清理死亡实体(要在dispatcher处理完事件后再清理，因此放在下一帧开头)
    remove_dead_system_->update(registry_);

    // 注意系统更新的顺序
    timer_system_->update(delta_time);
    game_rule_system_->update(delta_time);
    block_system_->update(registry_, dispatcher);
    set_target_system_->update(registry_);
    follow_path_system_->update(registry_, dispatcher, waypoint_nodes_);
    orientation_system_->update(registry_);  // 调用顺序要在Block、SetTarget、FollowPath之后
    attack_starter_system_->update(registry_, dispatcher);
    projectile_system_->update(delta_time);
    movement_system_->update(registry_, delta_time);
    animation_system_->update(delta_time);
    place_unit_system_->update(delta_time);
    ysort_system_->update(registry_);  // 调用顺序要在MovementSystem之后
    selection_system_->update();

    // 场景中其他更新函数
    // enemy_spawner_->update(delta_time);
    units_portrait_ui_->update(delta_time);
    Scene::update(delta_time);
}

void GameScene::render() {
    auto& renderer = context_.getRenderer();
    auto& camera = context_.getCamera();

    render_system_->update(registry_, renderer, camera);
    health_bar_system_->update(registry_, renderer, camera);
    render_range_system_->update(registry_, renderer, camera);

    Scene::render();
    debug_ui_system_->update();  // 调试UI的显示优先级最高，最后渲染
}

void GameScene::clean() {
    // 断开所有事件连接
    auto& dispatcher = context_.getDispatcher();
    dispatcher.disconnect(this);
    // 断开输入信号连接
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("pause"_hs).disconnect<&GameScene::onClearAllPlayers>(this);
    Scene::clean();
}

bool GameScene::initSessionData() {
    if (!session_data_) {
        session_data_ = std::make_shared<SessionData>();
        if (!session_data_->loadDefaultData()) {
            spdlog::error("初始化session_data_失败");
            return false;
        }
    }
    level_number_ = session_data_->getLevelNumber();
    return true;
}

bool GameScene::initLevelConfig() {
    if (!level_config_) {
        level_config_ = std::make_shared<LevelConfig>();
        if (!level_config_->loadFromFile("assets/data/level_config.json")) {
            spdlog::error("加载关卡配置失败");
            return false;
        }
    }
    waves_ = level_config_->getWavesData(level_number_);
    game_stats_.enemy_count_ = level_config_->getTotalEnemyCount(level_number_);
    return true;
}

bool GameScene::initUIConfig() {
    if (!ui_config_) {
        ui_config_ = std::make_shared<UIConfig>();
        if (!ui_config_->loadFromFile("assets/data/ui_config.json")) {
            spdlog::error("加载UI配置失败");
            return false;
        }
    }
    return true;
}

bool GameScene::loadLevel() {
    LevelLoader level_loader;
    // 设置拓展的构建器EntityBuilderMW
    level_loader.setEntityBuilder(
        std::make_unique<EntityBuilderMW>(level_loader, context_, registry_, waypoint_nodes_, start_points_));

    // 获取关卡地图路径
    auto map_path = level_config_->getMapPath(level_number_);
    if (!level_loader.loadLevel(map_path, this)) {
        spdlog::error("加载关卡失败");
        return false;
    }
    return true;
}

bool GameScene::initEventConnections() { return true; }

bool GameScene::initInputConnections() {
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("pause"_hs).connect<&GameScene::onClearAllPlayers>(this);
    return true;
}

bool GameScene::initEntityFactory() {
    // 如果蓝图管理器为空，则创建一个（将来可能由构造函数传入）
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
    entity_factory_ = std::make_unique<EntityFactory>(registry_, *blueprint_manager_);
    spdlog::info("entity_factory_ 加载完成");
    return true;
}

bool GameScene::initRegistryContext() {
    // 让注册表存储一些数据类型实例作为上下文，方便使用
    registry_.ctx().emplace<std::shared_ptr<BlueprintManager>>(blueprint_manager_);
    registry_.ctx().emplace<std::shared_ptr<SessionData>>(session_data_);
    registry_.ctx().emplace<std::shared_ptr<UIConfig>>(ui_config_);
    registry_.ctx().emplace<std::shared_ptr<LevelConfig>>(level_config_);
    registry_.ctx().emplace<std::unordered_map<int, WaypointNode>&>(waypoint_nodes_);
    registry_.ctx().emplace<std::vector<int>&>(start_points_);
    registry_.ctx().emplace<GameStats&>(game_stats_);
    registry_.ctx().emplace<Waves&>(waves_);
    registry_.ctx().emplace<int&>(level_number_);
    registry_.ctx().emplace_as<entt::entity&>("selected_unit"_hs, selected_unit_);
    registry_.ctx().emplace_as<entt::entity&>("hovered_unit"_hs, hovered_unit_);
    spdlog::info("registry_ 上下文初始化完成");
    return true;
}

bool GameScene::initSystems() {
    auto& dispatcher = context_.getDispatcher();
    // 系统初始化需要在可能的依赖模块(如实体工厂)初始化之后
    render_system_ = std::make_unique<RenderSystem>();
    movement_system_ = std::make_unique<MovementSystem>();
    animation_system_ = std::make_unique<AnimationSystem>(registry_, dispatcher);
    ysort_system_ = std::make_unique<YSortSystem>();
    audio_system_ = std::make_unique<AudioSystem>(registry_, context_);

    follow_path_system_ = std::make_unique<FollowPathSystem>();
    remove_dead_system_ = std::make_unique<RemoveDeadSystem>();
    block_system_ = std::make_unique<BlockSystem>();
    set_target_system_ = std::make_unique<SetTargetSystem>();
    attack_starter_system_ = std::make_unique<AttackStarterSystem>();
    timer_system_ = std::make_unique<TimerSystem>(registry_, dispatcher);
    orientation_system_ = std::make_unique<OrientationSystem>();
    animation_state_system_ = std::make_unique<AnimationStateSystem>(registry_, dispatcher);
    animation_event_system_ = std::make_unique<AnimationEventSystem>(registry_, dispatcher);
    combat_resolve_system_ = std::make_unique<CombatResolveSystem>(registry_, dispatcher);
    projectile_system_ = std::make_unique<ProjectileSystem>(registry_, dispatcher, *entity_factory_);
    effect_system_ = std::make_unique<EffectSystem>(registry_, dispatcher, *entity_factory_);
    health_bar_system_ = std::make_unique<HealthBarSystem>();
    game_rule_system_ = std::make_unique<GameRuleSystem>(registry_, dispatcher);
    place_unit_system_ = std::make_unique<PlaceUnitSystem>(registry_, *entity_factory_, context_);
    render_range_system_ = std::make_unique<RenderRangeSystem>();
    debug_ui_system_ = std::make_unique<DebugUISystem>(registry_, context_);
    selection_system_ = std::make_unique<SelectionSystem>(registry_, context_);
    skill_system_ = std::make_unique<SkillSystem>(registry_, dispatcher, *entity_factory_);

    spdlog::info("系统初始化完成");
    return true;
}

bool GameScene::initEnemySpawner() {
    enemy_spawner_ = std::make_unique<EnemySpawner>(registry_, *entity_factory_);
    spdlog::info("敌人生成器初始化完成");
    return true;
}

bool GameScene::initUnitsPortraitUI() {
    try {
        units_portrait_ui_ = std::make_unique<UnitsPortraitUI>(registry_, *ui_manager_, context_);
    } catch (const std::exception& e) {
        spdlog::error("初始化单位肖像UI失败: {}", e.what());
        return false;
    }
    return true;
}

bool GameScene::onClearAllPlayers() {
    auto view = registry_.view<PlayerComponent>();
    for (auto entity : view) {
        context_.getDispatcher().enqueue(RemovePlayerUnitEvent{entity});
    }
    return true;
}

}  // namespace pyc::monster_war