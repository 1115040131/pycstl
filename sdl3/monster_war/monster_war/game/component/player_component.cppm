module;

export module monster_war.game.component.player_component;

export namespace pyc::monster_war {

/// @brief 玩家组件，存储出击消耗
struct PlayerComponent {
    int cost_{};
};

}  // namespace pyc::monster_war
