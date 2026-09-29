module;

export module monster_war.game.component.blocker_component;

export namespace pyc::monster_war {

/// @brief 阻挡者组件，存储阻挡者最大阻挡数量和当前阻挡数量
struct BlockerComponent {
    int max_count_{};
    int current_count_{};
};

}  // namespace pyc::monster_war
