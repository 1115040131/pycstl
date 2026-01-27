#pragma once

#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>

namespace pyc::monster_war {

class Context;

class UIPortraitHoverEnterEvent;

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
    void update();

private:
    // 封装开始、结束帧的方法
    void beginFrame();
    void endFrame();

    // 封装每个UI显示模块
    void renderHoveredPortrait();
    void renderHoveredUnit();
    void renderSelectedUnit();
    void renderInfoUI();
    void renderSettingUI();
    void renderDebugUI();

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