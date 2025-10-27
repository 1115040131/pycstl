#pragma once

#include "sunny_land/engine/scene/scene.h"

namespace pyc::sunny_land {

class SessionData;

/**
 * @brief 游戏暂停时显示的菜单场景，提供继续、保存、返回、退出等选项。
 * 该场景通常被推送到 GameScene 之上。
 */
class MenuScene final : public Scene {
public:
    /**
     * @brief 构造函数
     * @param context 引擎上下文
     * @param scene_manager 场景管理器
     * @param session_data 指向游戏玩法状态的共享指针
     */
    MenuScene(Context& context, SceneManager& scene_manager, std::shared_ptr<SessionData> session_data = nullptr);

    // --- 核心方法 --- //
    void init() override;
    void handleInput() override;

private:
    // 初始化 UI 元素
    void createUI();

    // 按钮回调函数
    void onResumeClicked();
    void onSaveClicked();
    void onBackClicked();
    void onQuitClicked();

private:
    std::shared_ptr<SessionData> session_data_;
};

}  // namespace pyc::sunny_land