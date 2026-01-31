#pragma once

#include <chrono>

#include <entt/entity/fwd.hpp>
#include <entt/signal/fwd.hpp>

namespace pyc::monster_war {

class EnemyArriveHomeEvent;
class LevelClearDelayedEvent;
class UpgradeUnitEvent;
class RetreatEvent;

/**
 * @brief 游戏规则系统
 *
 * 负责处理游戏规则，如COST更新、敌人到达基地等。
 */
class GameRuleSystem {
public:
    GameRuleSystem(entt::registry& registry, entt::dispatcher& dispatcher);
    ~GameRuleSystem();

    void update(std::chrono::duration<float> delta_time);

private:
    // 事件回调函数
    void onEnemyArriveHome(const EnemyArriveHomeEvent& event);
    void onUpgradeUnitEvent(const UpgradeUnitEvent& event);
    void onRetreatEvent(const RetreatEvent& event);
    void onLevelClearDelayedEvent(const LevelClearDelayedEvent& event);

private:
    entt::registry& registry_;
    entt::dispatcher& dispatcher_;

    bool is_level_clear_{false};                            ///< @brief 是否关卡通关
    std::chrono::duration<float> level_clear_timer_{0.0f};  ///< @brief 关卡通关计时器(实现延迟切换场景)
};

}  // namespace pyc::monster_war