#pragma once

#include "monster_war/engine/scene/scene.h"
#include "monster_war/engine/system/fwd.h"
#include "monster_war/game/system/fwd.h"

namespace pyc::monster_war {

class BlueprintManager;
class LevelConfig;
class SessionData;
class UIConfig;

class TitleScene final : public Scene {
    friend class DebugUISystem;  // 允许DebugUISystem访问私有成员变量及方法

public:
    TitleScene(Context& context, std::shared_ptr<BlueprintManager> blueprint_manager = nullptr,
               std::shared_ptr<SessionData> session_data = nullptr, std::shared_ptr<UIConfig> ui_config = nullptr,
               std::shared_ptr<LevelConfig> level_config = nullptr);
    ~TitleScene() override;

    void init() override;
    void update(std::chrono::duration<float> delta_time) override;
    void render() override;

private:
    // 初始化函数(init函数中调用)
    [[nodiscard]] bool initSessionData();
    [[nodiscard]] bool initLevelConfig();
    [[nodiscard]] bool initBlueprintManager();
    [[nodiscard]] bool initUIConfig();
    [[nodiscard]] bool loadTitleLevel();
    [[nodiscard]] bool initSystems();
    [[nodiscard]] bool initRegistryContext();
    [[nodiscard]] bool initUI();

    // 按钮回调函数 (未来通过游戏UI调用)
    void onStartGameClick();
    void onConfirmRoleClick();
    void onLoadGameClick();
    void onQuitClick();

private:
    // 数据相关实例
    std::shared_ptr<BlueprintManager> blueprint_manager_;
    std::shared_ptr<SessionData> session_data_;
    std::shared_ptr<UIConfig> ui_config_;
    std::shared_ptr<LevelConfig> level_config_;

    // 系统相关实例
    std::unique_ptr<RenderSystem> render_system_;
    std::unique_ptr<YSortSystem> ysort_system_;
    std::unique_ptr<AnimationSystem> animation_system_;
    std::unique_ptr<MovementSystem> movement_system_;
    std::unique_ptr<DebugUISystem> debug_ui_system_;

    bool show_unit_info_{false};   ///< @brief 是否显示角色列表UI
    bool show_load_panel_{false};  ///< @brief 是否显示加载面板UI
};

}  // namespace pyc::monster_war