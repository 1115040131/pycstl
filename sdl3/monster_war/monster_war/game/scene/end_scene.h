#pragma once

#include "monster_war/engine/scene/scene.h"
#include "monster_war/game/system/fwd.h"

namespace pyc::monster_war {

class EndScene final : public Scene {
    friend class DebugUISystem;  // 允许DebugUISystem访问私有成员变量及方法

public:
    EndScene(Context& context, bool is_win = false);
    ~EndScene() override;

    void init() override;
    void render() override;

private:
    // 按钮回调函数
    void onBackToTitleClick();
    void onQuitClick();

private:
    // 目前只需要DebugUI系统
    std::unique_ptr<DebugUISystem> debug_ui_system_;

    bool is_win_{};  ///< @brief 是否显示保存面板
};

}  // namespace pyc::monster_war