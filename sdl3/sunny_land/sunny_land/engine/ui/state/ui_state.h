#pragma once

#include <memory>

#include "common/noncopyable.h"

namespace pyc::sunny_land {

class Context;

class UIInteractive;

/**
 * @brief 可交互UI元素在特定状态下的行为接口。
 *
 * 该接口定义了所有具体UI状态必须实现的通用操作，
 * 例如处理输入事件、更新状态逻辑以及确定视觉表现。
 */
class UIState : Noncopyable {
    friend class UIInteractive;

public:
    /**
     * @brief 构造函数传入父节点指针
     */
    explicit UIState(UIInteractive* owner) : owner_(owner) {}
    virtual ~UIState() = default;

protected:
    // --- 核心方法 ---
    virtual void enter() {}
    virtual std::unique_ptr<UIState> handleInput(Context& context) = 0;

protected:
    UIInteractive* owner_{};  ///< @brief 指向父节点
};

}  // namespace pyc::sunny_land