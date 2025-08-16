#pragma once

#include "common/noncopyable.h"

namespace pyc::sunny_land {

class GameObject;

/**
 * @brief 组件的抽象基类。
 *
 * 所有具体组件都应从此类继承。
 * 定义了组件生命周期中可能调用的通用方法。
 */
class Component : Noncopyable {
    friend class GameObject;  // 它需要调用Component的init方法

public:
    Component() = default;
    virtual ~Component() = default;

    void setOwner(GameObject* owner) { owner_ = owner; }  ///< @brief 设置拥有此组件的 GameObject
    GameObject* getOwner() const { return owner_; }       ///< @brief 获取拥有此组件的 GameObject

protected:
    // 关键循环函数，全部设为保护，只有 GameObject 需要（可以）调用
    virtual void init() {}         ///< @brief 保留两段初始化的机制，GameObject 添加组件时自动调用，不需要外部调用
    virtual void handleInput() {}  ///< @brief 处理输入
    virtual void update() {}       ///< @brief 更新，必须实现
    virtual void render() {}       ///< @brief 渲染
    virtual void clean() {}        ///< @brief 清理

protected:
    GameObject* owner_{};  ///< @brief 指向拥有此组件的 GameObject
};

}  // namespace pyc::sunny_land