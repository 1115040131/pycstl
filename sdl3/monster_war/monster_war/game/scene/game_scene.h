#pragma once

#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>

#include "monster_war/engine/scene/scene.h"
#include "monster_war/engine/system/fwd.h"
#include "monster_war/game/data/session_data.h"
#include "monster_war/game/data/waypoint_node.h"
#include "monster_war/game/def/events.h"
#include "monster_war/game/system/fwd.h"

namespace pyc::monster_war {

class BlueprintManager;
class EntityFactory;
class UIConfig;

class UIElement;

class GameScene final : public Scene {
public:
    GameScene(Context& context);
    ~GameScene() override;

    void init() override;
    void update(std::chrono::duration<float> delta_time) override;
    void render() override;
    void clean() override;

private:
    [[nodiscard]] bool initSessionData();
    [[nodiscard]] bool initUIConfig();
    [[nodiscard]] bool loadLevel();
    [[nodiscard]] bool initEventConnections();
    [[nodiscard]] bool initInputConnections();
    [[nodiscard]] bool initEntityFactory();
    [[nodiscard]] bool initSystems();

    void createUnitsPortraitUI();  ///< @brief 创建画面下方的单位肖像UI

    ///< @brief 排列画面下方的单位肖像UI (肖像增/减时调用)
    void arrangeUnitsPortraitUI(UIElement* anchor_panel, const glm::vec2& frame_size, float padding);

    // 事件回调函数
    void onEnemyArriveHome(const EnemyArriveHomeEvent& event);

    // 测试函数
    void testSessionData();
    void createTestEnemy();
    bool onCreateTestPlayerMelee();
    bool onCreateTestPlayerRanged();
    bool onCreateTestPlayerHealer();
    bool onClearAllPlayers();

private:
    std::unique_ptr<RenderSystem> render_system_;
    std::unique_ptr<MovementSystem> movement_system_;
    std::unique_ptr<AnimationSystem> animation_system_;
    std::unique_ptr<YSortSystem> ysort_system_;
    std::unique_ptr<AudioSystem> audio_system_;

    std::unique_ptr<FollowPathSystem> follow_path_system_;
    std::unique_ptr<RemoveDeadSystem> remove_dead_system_;
    std::unique_ptr<BlockSystem> block_system_;
    std::unique_ptr<SetTargetSystem> set_target_system_;
    std::unique_ptr<AttackStarterSystem> attack_starter_system_;
    std::unique_ptr<TimerSystem> timer_system_;
    std::unique_ptr<OrientationSystem> orientation_system_;
    std::unique_ptr<AnimationStateSystem> animation_state_system_;
    std::unique_ptr<AnimationEventSystem> animation_event_system_;
    std::unique_ptr<CombatResolveSystem> combat_resolve_system_;
    std::unique_ptr<ProjectileSystem> projectile_system_;
    std::unique_ptr<EffectSystem> effect_system_;
    std::unique_ptr<HealthBarSystem> health_bar_system_;

    std::unordered_map<int, WaypointNode> waypoint_nodes_;  // 路径节点ID到节点数据的映射
    std::vector<int> start_points_;                         // 起点ID列表

    std::unique_ptr<EntityFactory> entity_factory_;  // 实体工厂，负责创建和管理实体

    // 管理数据的实例很可能同时被多个场景使用，因此使用共享指针
    std::shared_ptr<BlueprintManager> blueprint_manager_;  // 蓝图管理器，负责管理蓝图数据
    std::shared_ptr<SessionData> session_data_;            // 会话数据，关卡切换时需要传递的数据
    std::shared_ptr<UIConfig> ui_config_;                  // UI配置，负责管理UI数据

    // --- 其他场景数据 ---
    int level_number_{1};
};

}  // namespace pyc::monster_war