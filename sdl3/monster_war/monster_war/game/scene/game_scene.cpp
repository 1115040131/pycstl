#include "monster_war/game/scene/game_scene.h"

#include <entt/core/hashed_string.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/component/render_component.h"
#include "monster_war/engine/component/sprite_component.h"
#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/component/velocity_component.h"
#include "monster_war/engine/core/context.h"
#include "monster_war/engine/loader/level_loader.h"
#include "monster_war/engine/system/animation_system.h"
#include "monster_war/engine/system/movement_system.h"
#include "monster_war/engine/system/render_system.h"
#include "monster_war/engine/system/ysort_system.h"
#include "monster_war/engine/ui/ui_manager.h"
#include "monster_war/engine/utils/events.h"
#include "monster_war/game/component/enemy_component.h"
#include "monster_war/game/loader/entity_builder_mw.h"
#include "monster_war/game/system/followpath_system.h"
#include "monster_war/game/system/remove_dead_system.h"

namespace pyc::monster_war {

using namespace entt::literals;

GameScene::GameScene(Context& context) : Scene("GameScene", context) {
    // 初始化系统
    render_system_ = std::make_unique<RenderSystem>();
    movement_system_ = std::make_unique<MovementSystem>();
    animation_system_ = std::make_unique<AnimationSystem>();
    ysort_system_ = std::make_unique<YSortSystem>();

    follow_path_system_ = std::make_unique<FollowPathSystem>();
    remove_dead_system_ = std::make_unique<RemoveDeadSystem>();

    spdlog::trace("GameScene 构造完成。");
}

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
    createTestEnemy();

    Scene::init();
}

void GameScene::update(std::chrono::duration<float> delta_time) {
    auto& dispatcher = context_.getDispatcher();

    // 每一帧最先清理死亡实体(要在dispatcher处理完事件后再清理，因此放在下一帧开头)
    remove_dead_system_->update(registry_);

    // 注意系统更新的顺序
    follow_path_system_->update(registry_, dispatcher, waypoint_nodes_);
    movement_system_->update(registry_, delta_time);
    animation_system_->update(registry_, delta_time);
    ysort_system_->update(registry_);  // 调用顺序要在MovementSystem之后

    Scene::update(delta_time);
}

void GameScene::render() {
    render_system_->update(registry_, context_.getRenderer(), context_.getCamera());

    Scene::render();
}

void GameScene::clean() {
    // 断开所有事件连接
    auto& dispatcher = context_.getDispatcher();
    dispatcher.disconnect(this);

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

void GameScene::onEnemyArriveHome(const EnemyArriveHomeEvent&) {
    spdlog::info("敌人到达基地");
    // TODO: 添加敌人到达基地的逻辑
}

void GameScene::createTestEnemy() {
    // 每个起点创建一个敌人
    for (auto start_index : start_points_) {
        auto enemy = registry_.create();

        registry_.emplace<TransformComponent>(enemy, waypoint_nodes_[start_index].position_);
        registry_.emplace<VelocityComponent>(enemy, glm::vec2(0.0f));
        registry_.emplace<EnemyComponent>(enemy, start_index, 100.0f);

        auto sprite = Sprite{
            "assets/textures/Enemy/wolf.png",
            Rect{
                glm::vec2(0.0f),
                glm::vec2(192.0f),
            },
        };
        // 设置精灵组件时，需设置偏移量以调整中心点位置(否则会默认以左上角为中心点)
        registry_.emplace<SpriteComponent>(enemy, std::move(sprite), glm::vec2(192, 192), glm::vec2(-96, -128));
        // 暂定主战斗图层编号为10
        registry_.emplace<RenderComponent>(enemy, 10);
    }
}

}  // namespace pyc::monster_war