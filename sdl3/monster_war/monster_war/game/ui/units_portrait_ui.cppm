module;

#include <chrono>

#include <entt/entity/fwd.hpp>
#include <glm/vec2.hpp>

export module monster_war.game.ui.units_portrait_ui;

export import monster_war.engine.core.context;
export import monster_war.engine.ui.ui_manager;
export import monster_war.engine.ui.ui_panel;
export import monster_war.game.def.events;

export namespace pyc::monster_war {

/**
 * @brief 单位肖像UI
 *
 * 负责管理单位肖像UI的创建、更新和排列。
 */
class UnitsPortraitUI {
public:
    /**
     * @brief 构造函数
     * @param registry 注册表
     * @param ui_manager UI管理器
     * @param context 引擎上下文
     */
    UnitsPortraitUI(entt::registry& registry, UIManager& ui_manager, Context& context);
    ~UnitsPortraitUI();

    void update(std::chrono::duration<float> delta_time);

    UIPanel* getAnchorPanel() const { return anchor_panel_; }

private:
    void updatePortraitCover();     ///< @brief 更新肖像遮盖
    void createUnitsPortraitUI();   ///< @brief 创建单位肖像UI
    void arrangeUnitsPortraitUI();  ///< @brief 排列单位肖像UI（肖像增/减时调用）

    void movePortraitPanelRight(std::chrono::duration<float> delta_time);  ///< @brief 向右移动单位肖像UI
    void movePortraitPanelLeft(std::chrono::duration<float> delta_time);   ///< @brief 向左移动单位肖像UI

    // 事件回调函数
    void onRemoveUIPortraitEvent(const RemoveUIPortraitEvent& event);

private:
    // --- 构造函数传入的外部组件引用 ---
    entt::registry& registry_;
    UIManager& ui_manager_;
    Context& context_;

    UIPanel* anchor_panel_;  ///< @brief 保存单位肖像UI的根面板(非拥有指针)，方便使用
};

}  // namespace pyc::monster_war
