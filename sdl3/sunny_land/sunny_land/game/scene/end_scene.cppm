module;

#include <memory>

export module sunny_land.game.scene:end_scene;

export import sunny_land.engine.scene;
export import sunny_land.game.data.session_data;

export namespace pyc::sunny_land {

/**
 * @class EndScene
 * @brief 显示游戏结束（胜利或失败）信息的叠加场景。
 *
 * 提供重新开始或返回主菜单的选项。
 */
class EndScene final : public Scene {
public:
    /**
     * @brief 构造函数
     * @param context 引擎上下文
     * @param scene_manager 场景管理器
     * @param session_data 指向游戏玩法状态的共享指针
     */
    EndScene(Context& context, SceneManager& scene_manager, std::shared_ptr<SessionData> session_data = nullptr);

    // --- 核心方法 --- //
    void init() override;

private:
    // 初始化 UI 元素
    void createUI();

    // 按钮回调函数
    void onBackClick();
    void onRestartClick();

private:
    std::shared_ptr<SessionData> session_data_;
};

}  // namespace pyc::sunny_land
