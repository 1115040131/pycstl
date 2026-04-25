#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>

namespace pyc::monster_war {

class Context;

class TitleScene;
class LevelClearScene;
class EndScene;

struct UIPortraitHoverEnterEvent;

/**
 * @brief 调试 UI 系统，负责显示调试 UI。
 *
 * @note 调试UI的主要目的是方便debug，并快速开发UI原型。
 * @note 游戏正式发布时往往会删除，因此不需要过度设计。
 */
class DebugUISystem {
public:
    DebugUISystem(entt::registry& registry, Context& context);
    ~DebugUISystem();

    // ImGui 步骤3: 一轮循环内，ImGui 需要做的操作（逻辑+渲染）
    void update();  ///<@brief 针对GameScene的更新
    ///<@brief 针对TitleScene的更新 (直接传入场景引用，提升便捷但增加耦合)
    void updateTitle(TitleScene& title_scene);
    void updateLevelClear(LevelClearScene& level_clear_scene);  ///<@brief 针对LevelClearScene的更新
    void updateEnd(EndScene& end_scene);                        ///<@brief 针对EndScene的更新

private:
    // 封装开始、结束帧的方法
    void beginFrame();
    void endFrame();

// 封装每个UI显示模块
#pragma region GameScene
    void renderHoveredPortrait();
    void renderHoveredUnit();
    void renderSelectedUnit();
    void renderInfoUI();
    void renderSettingUI();
    void renderDebugUI();
#pragma endregion

#pragma region TitleScene
    void renderTitleLogo();
    void renderTitleButtons(TitleScene& title_scene);
#pragma endregion

#pragma region LevelClearScene ---
    void renderLevelClearText();
    void renderLevelClearTable(LevelClearScene& level_clear_scene);
    void renderLevelClearButtons(LevelClearScene& level_clear_scene);
#pragma endregion

#pragma region EndScene ---
    void renderEndText(EndScene& end_scene);
    void renderEndButtons(EndScene& end_scene);
#pragma endregion

#pragma region Shared
    void renderUnitInfoUI(bool& show_unit_info);
    void renderSavePanelUI(bool& show_save_panel);
    void renderLoadPanelUI(bool& show_load_panel);
    void renderUnitTable();
#pragma endregion

    // 事件回调函数
    void onUIPortraitHoverEnterEvent(const UIPortraitHoverEnterEvent& event);
    void onUIPortraitHoverLeaveEvent();

private:
    entt::registry& registry_;
    Context& context_;

    entt::id_type hovered_portrait_{entt::null};  ///< @brief 悬浮肖像的角色名称ID
    bool show_debug_ui_{true};                    ///< @brief 是否显示调试UI
};

}  // namespace pyc::monster_war