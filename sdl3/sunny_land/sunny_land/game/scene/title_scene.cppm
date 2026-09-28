module;

#include <chrono>
#include <memory>

export module sunny_land.game.scene:title_scene;

export import sunny_land.engine.scene;
export import sunny_land.game.data.session_data;

export namespace pyc::sunny_land {

/**
 * @brief 标题场景类，提供4个按钮：开始游戏、加载游戏、帮助、退出
 */
class TitleScene final : public Scene {
public:
    /**
     * @brief 构造函数
     * @param context 引擎上下文
     * @param scene_manager 场景管理器
     * @param session_data 指向游戏玩法状态的共享指针
     */
    TitleScene(Context& context, SceneManager& scene_manager, std::shared_ptr<SessionData> session_data = nullptr);

    // --- 核心方法 --- //
    void init() override;
    void update(std::chrono::duration<float> delta_time) override;

private:
    // 初始化 UI 元素
    void createUI();

    // 按钮回调函数
    void onStartGameClick();
    void onLoadGameClick();
    void onHelpsClick();
    void onQuitClick();

private:
    std::shared_ptr<SessionData> session_data_;
};

}  // namespace pyc::sunny_land
