#include "sunny_land/engine/ui/ui_manager.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/ui/ui_panel.h"

namespace pyc::sunny_land {

UIManager::UIManager() {
    // 创建一个无特定大小和位置的Panel，它的子元素将基于它定位。
    root_element_ = std::make_unique<UIPanel>(glm::vec2{0.0f, 0.0f}, glm::vec2{0.0f, 0.0f});
    spdlog::trace("UI管理器构造完成。");
}

UIManager::~UIManager() = default;

bool UIManager::init(const glm::vec2& window_size) {
    root_element_->setSize(window_size);
    spdlog::trace("UI管理器已初始化根面板。");
    return true;
}

bool UIManager::handleInput(Context& context) {
    if (root_element_ && root_element_->isVisible()) {
        // 从根元素开始向下分发事件
        if (root_element_->handleInput(context)) return true;
    }
    return false;
}

void UIManager::update(std::chrono::duration<float> delta_time, Context& context) {
    if (root_element_ && root_element_->isVisible()) {
        // 从根元素开始向下更新
        root_element_->update(delta_time, context);
    }
}

void UIManager::render(Context& context) {
    if (root_element_ && root_element_->isVisible()) {
        // 从根元素开始向下渲染
        root_element_->render(context);
    }
}

void UIManager::addElement(std::unique_ptr<UIElement> element) {
    if (root_element_) {
        root_element_->addChild(std::move(element));
    } else {
        spdlog::error("无法添加元素: root_element_ 为空！");
    }
}

UIPanel* UIManager::getRootElement() const { return root_element_.get(); }

void UIManager::clearElements() {
    if (root_element_) {
        root_element_->removeAllChildren();
        spdlog::trace("所有UI元素已从UI管理器中清除。");
    }
}

}  // namespace pyc::sunny_land