#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include "common/noncopyable.h"

namespace pyc::sunny_land {

class Context;
class Scene;

/**
 * @brief 管理游戏中的场景栈，处理场景切换和生命周期。
 */
class SceneManager final : Noncopyable {
public:
    explicit SceneManager(Context& context);

    ~SceneManager();

    // getters
    Scene* getCurrentScene() const;                   ///< @brief 获取当前活动场景（栈顶场景）的指针。
    Context& getContext() const { return context_; }  ///< @brief 获取引擎上下文引用。

    // 核心循环函数
    void handleInput() const;
    void update(std::chrono::duration<double> delta_time);
    void render() const;
    void clean();

    // 延时切换场景
    void requestPushScene(std::unique_ptr<Scene> scene);     ///< @brief 请求压入一个新场景。
    void requestPopScene();                                  ///< @brief 请求弹出当前场景。
    void requestReplaceScene(std::unique_ptr<Scene> scene);  ///< @brief 请求替换当前场景。

private:
    void processPendingActions();  ///< @brief 处理挂起的场景操作（每轮更新最后调用）。

    // 直接切换场景
    void pushScene(std::unique_ptr<Scene> scene);     ///< @brief 将一个新场景压入栈顶，使其成为活动场景。
    void popScene();                                  ///< @brief 移除栈顶场景。
    void replaceScene(std::unique_ptr<Scene> scene);  ///< @brief 清理场景栈所有场景，将此场景设为栈顶场景。

private:
    Context& context_;                                 ///< @brief 引擎上下文引用
    std::vector<std::unique_ptr<Scene>> scene_stack_;  ///< @brief 场景栈

    enum class PendingAction { None, Push, Pop, Replace };  ///< @brief 待处理的动作
    PendingAction pending_action_ = PendingAction::None;    ///< @brief 待处理的动作
    std::unique_ptr<Scene> pending_scene_;                  ///< @brief 待处理场景
};

};  // namespace pyc::sunny_land