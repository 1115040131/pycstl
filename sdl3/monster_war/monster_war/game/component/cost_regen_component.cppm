module;

export module monster_war.game.component.cost_regen_component;

export namespace pyc::monster_war {

/// @brief COST恢复组件，其中COST会根据时间自动恢复
struct CostRegenComponent {
    float rate_{};  ///< @brief COST恢复速率/秒
};

}  // namespace pyc::monster_war
