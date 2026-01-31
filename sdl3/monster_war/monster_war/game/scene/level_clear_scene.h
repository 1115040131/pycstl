#pragma once

#include "monster_war/engine/scene/scene.h"
#include "monster_war/game/data/game_stats.h"
#include "monster_war/game/system/fwd.h"

namespace pyc::monster_war {

class BlueprintManager;
class LevelConfig;
class SessionData;
class UIConfig;

class LevelClearScene final : public Scene {
    friend class DebugUISystem;  // 允许DebugUISystem访问私有成员变量及方法

public:
    LevelClearScene(Context& context, GameStats& game_stats, std::shared_ptr<BlueprintManager> blueprint_manager,
                    std::shared_ptr<SessionData> session_data, std::shared_ptr<UIConfig> ui_config,
                    std::shared_ptr<LevelConfig> level_config);
    ~LevelClearScene() override;

    void init() override;
    void render() override;

private:
    // 按钮回调函数
    void onNextLevelClick();
    void onBackToTitleClick();
    void onSaveClick();

private:
    GameStats& game_stats_;  ///< @brief 构造函数传入关卡内游戏统计数据，需要在此场景中显示

    // 数据相关实例
    std::shared_ptr<BlueprintManager> blueprint_manager_;
    std::shared_ptr<SessionData> session_data_;
    std::shared_ptr<UIConfig> ui_config_;
    std::shared_ptr<LevelConfig> level_config_;

    // 目前只需要DebugUI系统
    std::unique_ptr<DebugUISystem> debug_ui_system_;

    bool show_save_panel_{false};  ///< @brief 是否显示保存面板
};

}  // namespace pyc::monster_war