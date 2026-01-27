#include "monster_war/game/system/game_rule_system.h"

#include <entt/entity/registry.hpp>
#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/component/transform_component.h"
#include "monster_war/engine/utils/events.h"
#include "monster_war/game/component/class_name_component.h"
#include "monster_war/game/component/cost_regen_component.h"
#include "monster_war/game/component/stats_component.h"
#include "monster_war/game/data/game_stats.h"
#include "monster_war/game/def/events.h"
#include "monster_war/game/factory/blueprint_manager.h"

namespace pyc::monster_war {

using namespace entt::literals;

GameRuleSystem::GameRuleSystem(entt::registry& registry, entt::dispatcher& dispatcher)
    : registry_(registry), dispatcher_(dispatcher) {
    dispatcher_.sink<EnemyArriveHomeEvent>().connect<&GameRuleSystem::onEnemyArriveHome>(this);
    dispatcher_.sink<UpgradeUnitEvent>().connect<&GameRuleSystem::onUpgradeUnitEvent>(this);
    dispatcher_.sink<RetreatEvent>().connect<&GameRuleSystem::onRetreatEvent>(this);
}

GameRuleSystem::~GameRuleSystem() { dispatcher_.disconnect(this); }

void GameRuleSystem::update(std::chrono::duration<float> delta_time) {
    // 更新Cost
    auto& game_stats = registry_.ctx().get<GameStats&>();
    game_stats.cost_ += game_stats.cost_gen_per_second_ * delta_time.count();
    // 更新COST恢复
    auto view_cost_regen = registry_.view<CostRegenComponent>();
    for (auto [_, cost_regen] : view_cost_regen.each()) {
        game_stats.cost_ += cost_regen.rate_ * delta_time.count();
    }
}

void GameRuleSystem::onEnemyArriveHome(const EnemyArriveHomeEvent&) {
    spdlog::info("敌人到达基地");
    auto& game_stats = registry_.ctx().get<GameStats&>();
    game_stats.enemy_arrived_count_++;  // 敌人到达数量+1
    game_stats.home_hp_ -= 1;           // 基地血量-1
    if (game_stats.home_hp_ <= 0) {
        spdlog::warn("基地被摧毁");
        // TODO: 切换场景逻辑
    }
}

void GameRuleSystem::onUpgradeUnitEvent(const UpgradeUnitEvent& event) {
    if (event.entity_ == entt::null || !registry_.valid(event.entity_)) {
        return;
    }
    // 扣除 cost
    auto& game_stats = registry_.ctx().get<GameStats&>();
    game_stats.cost_ -= event.cost_;
    // 等级提升
    auto& stats = registry_.get<StatsComponent>(event.entity_);
    stats.level_++;
    // 更新属性
    const auto& blueprint_manager = registry_.ctx().get<std::shared_ptr<BlueprintManager>>();
    const auto& class_name = registry_.get<ClassNameComponent>(event.entity_);
    const auto& stats_blueprint = blueprint_manager->getPlayerClassBlueprint(class_name.class_id_).stats_;
    stats.hp_ = statModify(stats_blueprint.hp_, stats.level_, stats.rarity_);
    stats.max_hp_ = statModify(stats_blueprint.hp_, stats.level_, stats.rarity_);
    stats.atk_ = statModify(stats_blueprint.atk_, stats.level_, stats.rarity_);
    stats.def_ = statModify(stats_blueprint.def_, stats.level_, stats.rarity_);
    // 创建特效
    const auto& transform = registry_.get<TransformComponent>(event.entity_);
    dispatcher_.enqueue(EffectEvent{"level_up"_hs, transform.position_, false});
    // 播放音效
    dispatcher_.enqueue(PlaySoundEvent{event.entity_, "level_up"_hs});
}

void GameRuleSystem::onRetreatEvent(const RetreatEvent& event) {
    if (event.entity_ == entt::null || !registry_.valid(event.entity_)) {
        return;
    }
    // 返还 cost
    auto& game_stats = registry_.ctx().get<GameStats&>();
    game_stats.cost_ += event.cost_;
    // 发送移除单位事件
    dispatcher_.enqueue(RemovePlayerUnitEvent{event.entity_});
}

}  // namespace pyc::monster_war