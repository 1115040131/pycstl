#include "monster_war/engine/scene/scene_manager.h"

#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/core/context.h"
#include "monster_war/engine/scene/scene.h"
#include "monster_war/engine/utils/events.h"

namespace pyc::monster_war {

SceneManager::SceneManager(Context& context) : context_(context) {
    // 注册事件处理函数
    context_.getDispatcher().sink<PopSceneEvent>().connect<&SceneManager::onPopScene>(this);
    context_.getDispatcher().sink<PushSceneEvent>().connect<&SceneManager::onPushScene>(this);
    context_.getDispatcher().sink<ReplaceSceneEvent>().connect<&SceneManager::onReplaceScene>(this);
    spdlog::trace("场景管理器已创建。");
}

SceneManager::~SceneManager() {
    spdlog::trace("场景管理器已销毁。");
    clean();  // 即使不手动调用 clean 也能确保清理
}

Scene* SceneManager::getCurrentScene() const {
    if (scene_stack_.empty()) {
        return nullptr;
    }
    return scene_stack_.back().get();
}

void SceneManager::handleInput() const {  // 只考虑栈顶场景
    if (auto current_scene = getCurrentScene()) {
        current_scene->handleInput();
    }
}

void SceneManager::update(std::chrono::duration<float> delta_time) {
    // 只更新栈顶（当前）场景
    if (auto current_scene = getCurrentScene()) {
        current_scene->update(delta_time);
    }
    // 执行可能的切换场景操作
    processPendingActions();
}

void SceneManager::render() const {
    // 渲染时需要叠加渲染所有场景，而不只是栈顶
    for (const auto& scene : scene_stack_) {
        if (scene) {
            scene->render();
        }
    }
}

void SceneManager::clean() {
    spdlog::trace("正在关闭场景管理器并清理场景栈...");
    // 清理栈中所有剩余的场景（从顶到底）
    while (!scene_stack_.empty()) {
        if (scene_stack_.back()) {
            spdlog::debug("正在清理场景 '{}' 。", scene_stack_.back()->getName());
            scene_stack_.back()->clean();
        }
        scene_stack_.pop_back();
    }
    // 断开事件处理函数 (一次性断开所有和当前实例绑定的回调函数)
    context_.getDispatcher().disconnect(this);
}

void SceneManager::onPopScene() { pending_action_ = PendingAction::Pop; }

void SceneManager::onPushScene(PushSceneEvent& event) {
    pending_action_ = PendingAction::Push;
    pending_scene_ = std::move(event.scene);
}

void SceneManager::onReplaceScene(ReplaceSceneEvent& event) {
    pending_action_ = PendingAction::Replace;
    pending_scene_ = std::move(event.scene);
}

void SceneManager::processPendingActions() {
    switch (pending_action_) {
        case PendingAction::None:
            return;
        case PendingAction::Push:
            pushScene(std::move(pending_scene_));
            break;
        case PendingAction::Pop:
            popScene();
            break;
        case PendingAction::Replace:
            replaceScene(std::move(pending_scene_));
            break;
    }
    pending_action_ = PendingAction::None;
}

void SceneManager::pushScene(std::unique_ptr<Scene> scene) {
    if (!scene) {
        spdlog::warn("尝试将空场景压入栈。");
        return;
    }
    spdlog::debug("正在将场景 '{}' 压入栈。", scene->getName());

    // 初始化新场景
    if (!scene->isInitialized()) {  // 确保只初始化一次
        scene->init();
    }

    // 将新场景移入栈顶
    scene_stack_.push_back(std::move(scene));
}

void SceneManager::popScene() {
    if (scene_stack_.empty()) {
        spdlog::warn("尝试从空场景栈中弹出。");
        return;
    }
    spdlog::debug("正在从栈中弹出场景 '{}' 。", scene_stack_.back()->getName());

    // 清理并移除栈顶场景
    if (scene_stack_.back()) {
        scene_stack_.back()->clean();  // 显式调用清理
    }
    scene_stack_.pop_back();
    if (scene_stack_.empty()) {
        spdlog::warn("弹出最后一个场景, 退出游戏。");
        context_.getDispatcher().enqueue<QuitEvent>();
    }
}

void SceneManager::replaceScene(std::unique_ptr<Scene> scene) {
    if (!scene) {
        spdlog::warn("尝试用空场景替换。");
        return;
    }
    spdlog::debug("正在用场景 '{}' 替换场景 '{}' 。", scene->getName(), scene_stack_.back()->getName());

    // 清理栈中所有剩余的场景（从顶到底）
    while (!scene_stack_.empty()) {
        if (scene_stack_.back()) {
            spdlog::debug("正在清理场景 '{}' 。", scene_stack_.back()->getName());
            scene_stack_.back()->clean();
        }
        scene_stack_.pop_back();
    }

    // 压入新场景
    pushScene(std::move(scene));
}

}  // namespace pyc::monster_war