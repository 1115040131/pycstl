#include "sunny_land/engine/scene/scene.h"

#include <spdlog/spdlog.h>

#include "sunny_land/engine/core/context.h"
#include "sunny_land/engine/object/game_object.h"
#include "sunny_land/engine/physics/physics_engine.h"

namespace pyc::sunny_land {

Scene::Scene(std::string_view name, Context& context, SceneManager& scene_manager)
    : scene_name_(name), context_(context), scene_manager_(scene_manager) {
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

    for (const auto& game_object : game_objects_) {
        game_object->handleInput(context_);
    }
}

void Scene::update(std::chrono::duration<float> delta_time) {
    if (!is_initialized_) {
        return;
    }

    // 先更新物理引擎
    context_.getPhysicsEngine().update(delta_time);

    auto partition_it = std::partition(game_objects_.begin(), game_objects_.end(),
                                       [delta_time, this](const std::unique_ptr<GameObject>& game_object) {
                                           if (game_object && !game_object->isNeedRemove()) {
                                               game_object->update(delta_time, context_);
                                               return true;
                                           } else {
                                               if (game_object) {
                                                   game_object->clean();
                                               }
                                               return false;
                                           }
                                       });
    game_objects_.erase(partition_it, game_objects_.end());

    processPendingAdditions();
}

void Scene::render() {
    if (!is_initialized_) {
        return;
    }

    for (const auto& game_object : game_objects_) {
        game_object->render(context_);
    }
}

void Scene::clean() {
    if (!is_initialized_) {
        return;
    }

    for (const auto& game_object : game_objects_) {
        if (game_object) {
            game_object->clean();
        }
    }
    game_objects_.clear();

    is_initialized_ = false;  // 清理完成后，设置场景为未初始化
    spdlog::trace("场景 '{}' 清理完成。", scene_name_);
}

void Scene::addGameObject(std::unique_ptr<GameObject> game_object) {
    if (game_object) {
        game_objects_.push_back(std::move(game_object));
    } else {
        spdlog::warn("尝试向场景 '{}' 添加空游戏对象。", scene_name_);
    }
}

void Scene::safeAddGameObject(std::unique_ptr<GameObject> game_object) {
    if (game_object) {
        pending_additions_.push_back(std::move(game_object));
    } else {
        spdlog::warn("尝试向场景 '{}' 添加空游戏对象。", scene_name_);
    }
}

void Scene::removeGameObject(GameObject* game_object_ptr) {
    if (!game_object_ptr) {
        spdlog::warn("尝试从场景 '{}' 中移除一个空的游戏对象指针。", scene_name_);
        return;
    }

    auto result = std::erase_if(game_objects_, [game_object_ptr](const std::unique_ptr<GameObject>& game_object) {
        return game_object.get() == game_object_ptr;
    });
    if (result == 0) {
        spdlog::warn("在场景 '{}' 中未找到要移除的游戏对象 '{}'.", scene_name_, game_object_ptr->getName());
    } else {
        spdlog::trace("从场景 '{}' 中移除游戏对象 '{}'.", scene_name_, game_object_ptr->getName());
    }
}

void Scene::safeRemoveGameObject(GameObject* game_object_ptr) const { game_object_ptr->setNeedRemove(true); }

GameObject* Scene::findGameObjectByName(std::string_view name) const {
    for (const auto& game_object : game_objects_) {
        if (game_object->getName() == name) {
            return game_object.get();
        }
    }
    return nullptr;
}

void Scene::processPendingAdditions() {
    for (auto& game_object : pending_additions_) {
        addGameObject(std::move(game_object));
    }
    pending_additions_.clear();
}

}  // namespace pyc::sunny_land