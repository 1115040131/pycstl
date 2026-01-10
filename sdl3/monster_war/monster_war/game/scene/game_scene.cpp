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
#include "monster_war/engine/ui/ui_manager.h"
#include "monster_war/engine/utils/events.h"
#include "monster_war/game/component/player_component.h"
#include "monster_war/game/component/stats_component.h"
#include "monster_war/game/def/tag.h"
#include "monster_war/game/factory/blueprint_manager.h"
#include "monster_war/game/factory/entity_factory.h"
#include "monster_war/game/loader/entity_builder_mw.h"
#include "monster_war/game/system/animation_event_system.h"
#include "monster_war/game/system/animation_state_system.h"
#include "monster_war/game/system/attack_starter_system.h"
#include "monster_war/game/system/block_system.h"
#include "monster_war/game/system/combat_resolve_system.h"
#include "monster_war/game/system/effect_system.h"
#include "monster_war/game/system/followpath_system.h"
#include "monster_war/game/system/health_bar_system.h"
#include "monster_war/game/system/orientation_system.h"
#include "monster_war/game/system/projectile_system.h"
#include "monster_war/game/system/remove_dead_system.h"
#include "monster_war/game/system/set_target_system.h"
#include "monster_war/game/system/timer_system.h"

namespace pyc::monster_war {

using namespace entt::literals;

GameScene::GameScene(Context& context) : Scene("GameScene", context) { spdlog::trace("GameScene 构造完成。"); }

GameScene::~GameScene() = default;

void GameScene::init() {
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
    if (!initSystems()) {
        spdlog::error("初始化系统失败");
        return;
    }

    createTestEnemy();

    Scene::init();
}

void GameScene::update(std::chrono::duration<float> delta_time) {
    auto& dispatcher = context_.getDispatcher();

    // 每一帧最先清理死亡实体(要在dispatcher处理完事件后再清理，因此放在下一帧开头)
    remove_dead_system_->update(registry_);

    // 注意系统更新的顺序
    timer_system_->update(registry_, delta_time);
    block_system_->update(registry_, dispatcher);
    set_target_system_->update(registry_);
    follow_path_system_->update(registry_, dispatcher, waypoint_nodes_);
    orientation_system_->update(registry_);  // 调用顺序要在Block、SetTarget、FollowPath之后
    attack_starter_system_->update(registry_, dispatcher);
    projectile_system_->update(delta_time);
    movement_system_->update(registry_, delta_time);
    animation_system_->update(delta_time);
    ysort_system_->update(registry_);  // 调用顺序要在MovementSystem之后

    Scene::update(delta_time);
}

void GameScene::render() {
    auto& renderer = context_.getRenderer();
    auto& camera = context_.getCamera();

    render_system_->update(registry_, renderer, camera);
    health_bar_system_->update(registry_, renderer, camera);

    Scene::render();
}

void GameScene::clean() {
    // 断开所有事件连接
    auto& dispatcher = context_.getDispatcher();
    dispatcher.disconnect(this);
    // 断开输入信号连接
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("mouse_right"_hs).disconnect<&GameScene::onCreateTestPlayerMelee>(this);
    input_manager.onAction("mouse_left"_hs).disconnect<&GameScene::onCreateTestPlayerRanged>(this);
    input_manager.onAction("pause"_hs).disconnect<&GameScene::onClearAllPlayers>(this);
    input_manager.onAction("move_left"_hs).disconnect<&GameScene::onCreateTestPlayerHealer>(this);
    Scene::clean();
}

bool GameScene::loadLevel() {
    LevelLoader level_loader;
    // 设置拓展的构建器EntityBuilderMW
    level_loader.setEntityBuilder(
        std::make_unique<EntityBuilderMW>(level_loader, context_, registry_, waypoint_nodes_, start_points_));

    if (!level_loader.loadLevel("assets/maps/level1.tmj", this)) {
        spdlog::error("加载关卡失败");
        return false;
    }
    return true;
}

bool GameScene::initEventConnections() {
    auto& dispatcher = context_.getDispatcher();
    dispatcher.sink<EnemyArriveHomeEvent>().connect<&GameScene::onEnemyArriveHome>(this);
    return true;
}

bool GameScene::initInputConnections() {
    auto& input_manager = context_.getInputManager();
    input_manager.onAction("mouse_right"_hs).connect<&GameScene::onCreateTestPlayerMelee>(this);
    input_manager.onAction("mouse_left"_hs).connect<&GameScene::onCreateTestPlayerRanged>(this);
    input_manager.onAction("pause"_hs).connect<&GameScene::onClearAllPlayers>(this);
    input_manager.onAction("move_left"_hs).connect<&GameScene::onCreateTestPlayerHealer>(this);
    return true;
}

bool GameScene::initEntityFactory() {
    // 如果蓝图管理器为空，则创建一个（将来可能由构造函数传入）
    if (!blueprint_manager_) {
        blueprint_manager_ = std::make_shared<BlueprintManager>(context_.getResourceManager());
        if (!blueprint_manager_->loadEnemyClassBlueprints("assets/data/enemy_data.json") ||
            !blueprint_manager_->loadPlayerClassBlueprints("assets/data/player_data.json") ||
            !blueprint_manager_->loadProjectileBlueprints("assets/data/projectile_data.json")) {
            spdlog::error("加载蓝图失败");
            return false;
        }
    }
    entity_factory_ = std::make_unique<EntityFactory>(registry_, *blueprint_manager_);
    spdlog::info("entity_factory_ 加载完成");
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
    timer_system_ = std::make_unique<TimerSystem>();
    orientation_system_ = std::make_unique<OrientationSystem>();
    animation_state_system_ = std::make_unique<AnimationStateSystem>(registry_, dispatcher);
    animation_event_system_ = std::make_unique<AnimationEventSystem>(registry_, dispatcher);
    combat_resolve_system_ = std::make_unique<CombatResolveSystem>(registry_, dispatcher);
    projectile_system_ = std::make_unique<ProjectileSystem>(registry_, dispatcher, *entity_factory_);
    effect_system_ = std::make_unique<EffectSystem>(registry_, dispatcher, *entity_factory_);
    health_bar_system_ = std::make_unique<HealthBarSystem>();

    spdlog::info("系统初始化完成");
    return true;
}

void GameScene::onEnemyArriveHome(const EnemyArriveHomeEvent&) {
    spdlog::info("敌人到达基地");
    // TODO: 添加敌人到达基地的逻辑
}

void GameScene::createTestEnemy() {
    // 每个起点创建一个敌人
    for (auto start_index : start_points_) {
        auto position = waypoint_nodes_[start_index].position_;

        entity_factory_->createEnemyUnit("wolf"_hs, position, start_index);
        entity_factory_->createEnemyUnit("slime"_hs, position, start_index);
        entity_factory_->createEnemyUnit("goblin"_hs, position, start_index);
        entity_factory_->createEnemyUnit("dark_witch"_hs, position, start_index);
    }
}

bool GameScene::onCreateTestPlayerMelee() {
    auto position = context_.getInputManager().getLogicalMousePosition();
    auto entity = entity_factory_->createPlayerUnit("warrior"_hs, position);
    // 让玩家处于受伤状态（治疗师不会锁定满血目标）
    registry_.emplace<InjuredTag>(entity);
    auto& stats = registry_.get<StatsComponent>(entity);
    stats.hp_ = stats.max_hp_ / 2;
    spdlog::info("创建战士: 位置: {}, {}", position.x, position.y);
    return true;
}

bool GameScene::onCreateTestPlayerRanged() {
    auto position = context_.getInputManager().getLogicalMousePosition();
    auto entity = entity_factory_->createPlayerUnit("archer"_hs, position);
    // 让玩家处于受伤状态（治疗师不会锁定满血目标）
    registry_.emplace<InjuredTag>(entity);
    auto& stats = registry_.get<StatsComponent>(entity);
    stats.hp_ = stats.max_hp_ / 2;
    spdlog::info("创建弓箭手: 位置: {}, {}", position.x, position.y);
    return true;
}

bool GameScene::onCreateTestPlayerHealer() {
    auto position = context_.getInputManager().getLogicalMousePosition();
    entity_factory_->createPlayerUnit("witch"_hs, position);
    spdlog::info("创建治疗者: 位置: {}, {}", position.x, position.y);
    return true;
}

bool GameScene::onClearAllPlayers() {
    auto view = registry_.view<PlayerComponent>();
    for (auto entity : view) {
        registry_.destroy(entity);
    }
    return true;
}

}  // namespace pyc::monster_war