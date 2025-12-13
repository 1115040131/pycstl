#include "monster_war/engine/scene/scene.h"

#include <entt/signal/dispatcher.hpp>
#include <spdlog/spdlog.h>

#include "monster_war/engine/core/context.h"
#include "monster_war/engine/ui/ui_manager.h"
#include "monster_war/engine/utils/events.h"

namespace pyc::monster_war {

Scene::Scene(std::string_view name, Context& context)
    : scene_name_(name), context_(context), ui_manager_(std::make_unique<UIManager>()) {
    spdlog::trace("场景 '{}' 构造完成。", scene_name_);
}

Scene::~Scene() = default;

void Scene::init() {
    is_initialized_ = true;  // 子类应该最后调用父类的 init 方法
    spdlog::trace("场景 '{}' 初始化完成。", scene_name_);
}

void Scene::handleInput() {
    if (!is_initialized_) {
        return;
    }

    // 处理UI管理器输入
    if (ui_manager_->handleInput(context_)) {
        return;  // 如果输入事件被UI处理则返回，不再处理游戏对象输入
    }
}

void Scene::update(std::chrono::duration<float> delta_time) {
    if (!is_initialized_) {
        return;
    }

    // 更新UI管理器
    ui_manager_->update(delta_time, context_);
}

void Scene::render() {
    if (!is_initialized_) {
        return;
    }

    // 渲染UI管理器
    ui_manager_->render(context_);
}

void Scene::clean() {
    if (!is_initialized_) {
        return;
    }

    registry_.clear();        // 清理ECS注册表
    is_initialized_ = false;  // 清理完成后，设置场景为未初始化
    spdlog::trace("场景 '{}' 清理完成。", scene_name_);
}

void Scene::requestPopScene() { context_.getDispatcher().trigger<PopSceneEvent>(); }

void Scene::requestPushScene(std::unique_ptr<Scene> scene) {
    context_.getDispatcher().trigger<PushSceneEvent>(PushSceneEvent{std::move(scene)});
}

void Scene::requestReplaceScene(std::unique_ptr<Scene> scene) {
    context_.getDispatcher().trigger<ReplaceSceneEvent>(ReplaceSceneEvent{std::move(scene)});
}

void Scene::quit() { context_.getDispatcher().trigger<QuitEvent>(); }

}  // namespace pyc::monster_war