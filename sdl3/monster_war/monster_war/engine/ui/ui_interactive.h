#pragma once

#include <string_view>
#include <unordered_map>

#include <entt/entity/fwd.hpp>

#include "monster_war/engine/ui/ui_element.h"

namespace pyc::monster_war {

class Context;
class Image;
class UIState;

/**
 * @brief 可交互UI元素的基类,继承自UIElement
 *
 * 定义了可交互UI元素的通用属性和行为。
 * 管理UI状态的切换和交互逻辑。
 * 提供事件处理、更新和渲染的虚方法。
 */
class UIInteractive : public UIElement {
public:
    explicit UIInteractive(Context& context, glm::vec2 position = {0.0f, 0.0f}, glm::vec2 size = {0.0f, 0.0f});
    ~UIInteractive() override;

    virtual void clicked() {}      ///< @brief 如果有点击事件，则重写该方法
    virtual void hover_enter() {}  ///< @brief 如果有悬停进入事件，则重写该方法
    virtual void hover_leave() {}  ///< @brief 如果有悬停离开事件，则重写该方法

    void addImage(entt::id_type name_id, Image image);  ///< @brief 添加/替换图片
    void setCurrentImage(entt::id_type name_id);        ///< @brief 设置当前显示的图片

    void setHoverSound(entt::id_type id, std::string_view path = "");  ///< @brief 设置悬浮音效
    void setClickSound(entt::id_type id, std::string_view path = "");  ///< @brief 设置点击音效
    void playSound(entt::id_type name_id);                             ///< @brief 播放音效

    // --- Getters and Setters ---
    Context& getContext() const { return context_; }

    void setState(std::unique_ptr<UIState> state);      ///< @brief 设置当前状态
    void setNextState(std::unique_ptr<UIState> state);  ///< @brief 设置下一个状态
    UIState* getState() const { return state_.get(); }  ///< @brief 获取当前状态

    void setInteractive(bool interactive) { interactive_ = interactive; }  ///< @brief 设置是否可交互
    bool isInteractive() const { return interactive_; }                    ///< @brief 获取是否可交互

    // --- 核心方法 ---
    void update(std::chrono::duration<float> delta_time, Context& context) override;
    void render(Context& context) override;

protected:
    Context& context_;                                         ///< @brief 可交互元素很可能需要其他引擎组件
    std::unique_ptr<UIState> state_;                           ///< @brief 当前状态
    std::unique_ptr<UIState> next_state_;                      ///< @brief 下一个状态，用于处理状态切换
    std::unordered_map<entt::id_type, Image> images_;          ///< @brief 图片集合
    std::unordered_map<entt::id_type, entt::id_type> sounds_;  ///< @brief 音效集合，key为音效名称ID，value为音效ID
    entt::id_type current_image_id_ = entt::null;              ///< @brief 当前显示的图片ID
    bool interactive_ = true;                                  ///< @brief 是否可交互
};

}  // namespace pyc::monster_war