module;

#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>

#include <entt/entity/entity.hpp>

export module monster_war.game.scene:game_scene;

import :debug_ui_system;
export import monster_war.engine.core.context;
export import monster_war.engine.scene.scene;
import monster_war.engine.system.animation_system;
import monster_war.engine.system.audio_system;
import monster_war.engine.system.movement_system;
import monster_war.engine.system.render_system;
import monster_war.engine.system.ysort_system;
export import monster_war.game.data.game_stats;
export import monster_war.game.data.level_config;
export import monster_war.game.data.level_data;
export import monster_war.game.data.session_data;
export import monster_war.game.data.ui_config;
export import monster_war.game.data.waypoint_node;
export import monster_war.game.def.events;
export import monster_war.game.factory.blueprint_manager;
export import monster_war.game.factory.entity_factory;
export import monster_war.game.spawner.enemy_spawner;
import monster_war.game.system.animation_event_system;
import monster_war.game.system.animation_state_system;
import monster_war.game.system.attack_starter_system;
import monster_war.game.system.block_system;
import monster_war.game.system.combat_resolve_system;
import monster_war.game.system.effect_system;
import monster_war.game.system.followpath_system;
import monster_war.game.system.game_rule_system;
import monster_war.game.system.health_bar_system;
import monster_war.game.system.orientation_system;
import monster_war.game.system.place_unit_system;
import monster_war.game.system.projectile_system;
import monster_war.game.system.remove_dead_system;
import monster_war.game.system.render_range_system;
import monster_war.game.system.selection_system;
import monster_war.game.system.set_target_system;
import monster_war.game.system.skill_system;
import monster_war.game.system.timer_system;
export import monster_war.game.ui.units_portrait_ui;

export namespace pyc::monster_war {

class GameScene final : public Scene {
public:
    /**
     * @brief 构造函数
     * @param context 上下文
     * @param blueprint_manager 蓝图管理器
     * @param session_data 场景间传递的关卡数据
     * @param ui_config UI配置
     * @param level_config 关卡配置
     */
    GameScene(Context& context, std::shared_ptr<BlueprintManager> blueprint_manager = nullptr,
              std::shared_ptr<SessionData> session_data = nullptr, std::shared_ptr<UIConfig> ui_config = nullptr,
              std::shared_ptr<LevelConfig> level_config = nullptr);
    ~GameScene() override;

    void init() override;
    void update(std::chrono::duration<float> delta_time) override;
    void render() override;
    void clean() override;

private:
    [[nodiscard]] bool initSessionData();
    [[nodiscard]] bool initLevelConfig();
    [[nodiscard]] bool initUIConfig();
    [[nodiscard]] bool loadLevel();
    [[nodiscard]] bool initEventConnections();
    [[nodiscard]] bool initInputConnections();
    [[nodiscard]] bool initEntityFactory();
    [[nodiscard]] bool initRegistryContext();
    [[nodiscard]] bool initSystems();
    [[nodiscard]] bool initEnemySpawner();
    [[nodiscard]] bool initUnitsPortraitUI();

    // 场景相关函数
    void onRestart();
    void onBackToTitle();
    void onSave();
    void onLevelClear();
    void onGameEndEvent(const GameEndEvent& event);

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
    std::unique_ptr<GameRuleSystem> game_rule_system_;
    std::unique_ptr<PlaceUnitSystem> place_unit_system_;
    std::unique_ptr<RenderRangeSystem> render_range_system_;
    std::unique_ptr<DebugUISystem> debug_ui_system_;
    std::unique_ptr<SelectionSystem> selection_system_;
    std::unique_ptr<SkillSystem> skill_system_;

    std::unique_ptr<EnemySpawner> enemy_spawner_;         // 敌人生成器，负责生成敌人
    std::unique_ptr<UnitsPortraitUI> units_portrait_ui_;  // 封装的单位肖像UI，负责管理单位肖像UI的创建、更新和排列

    std::unordered_map<int, WaypointNode> waypoint_nodes_;  // 路径节点ID到节点数据的映射
    std::vector<int> start_points_;                         // 起点ID列表
    GameStats game_stats_;                                  // 关卡内游戏统计数据
    Waves waves_;                                           // 关卡波次数据

    std::unique_ptr<EntityFactory> entity_factory_;  // 实体工厂，负责创建和管理实体

    // 管理数据的实例很可能同时被多个场景使用，因此使用共享指针
    std::shared_ptr<BlueprintManager> blueprint_manager_;  // 蓝图管理器，负责管理蓝图数据
    std::shared_ptr<SessionData> session_data_;            // 会话数据，关卡切换时需要传递的数据
    std::shared_ptr<UIConfig> ui_config_;                  // UI配置，负责管理UI数据
    std::shared_ptr<LevelConfig> level_config_;            // 关卡配置，负责管理关卡数据

    // --- 其他场景数据 ---
    int level_number_{1};
    entt::entity selected_unit_{entt::null};  // 游戏中鼠标选中的单位
    entt::entity hovered_unit_{entt::null};   // 游戏中鼠标悬浮的单位
    bool show_save_panel_{false};             // 是否显示保存面板
};

}  // namespace pyc::monster_war
