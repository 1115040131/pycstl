#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

void Scene::clean() {
    Object::clean();
    for (auto& child : children_world_) {
        child->clean();
    }
    children_world_.clear();
    for (auto& child : children_screen_) {
        child->clean();
    }
    children_screen_.clear();
}

void Scene::handleEvents(SDL_Event& event) {
    Object::handleEvents(event);
    for (auto& child : children_screen_) {
        if (child->isActive()) {
            child->handleEvents(event);
        }
    }
    for (auto& child : children_world_) {
        if (child->isActive()) {
            child->handleEvents(event);
        }
    }
}

void Scene::update(std::chrono::duration<float> delta) {
    Object::update(delta);
    for (auto& child : children_world_) {
        if (child->isActive()) {
            child->update(delta);
        }
    }
    for (auto& child : children_screen_) {
        if (child->isActive()) {
            child->update(delta);
        }
    }
}

void Scene::render() {
    Object::render();
    for (auto& child : children_world_) {
        if (child->isActive()) {
            child->render();
        }
    }
    for (auto& child : children_screen_) {
        if (child->isActive()) {
            child->render();
        }
    }
}

void Scene::setCameraPosition(const glm::vec2& camera_position) {
    constexpr auto kOffset = glm::vec2(30.F);
    camera_position_ = glm::clamp(camera_position, -kOffset, world_size_ - game_.getScreenSize() + kOffset);
}

void Scene::addChild(std::unique_ptr<Object> child) {
    switch (child->getType()) {
        case Object::Type::kWorld:
            children_world_.emplace_back(static_cast<ObjectWorld*>(child.release()));
            break;
        case Object::Type::kScreen:
            children_screen_.emplace_back(static_cast<ObjectScreen*>(child.release()));
            break;
        default:
            children_.push_back(std::move(child));
            break;
    }
}

void Scene::removeChild(Object* child_to_remove) {
    switch (child_to_remove->getType()) {
        case Object::Type::kWorld:
            std::erase_if(children_world_, [child_to_remove](const std::unique_ptr<ObjectWorld>& child) {
                return child.get() == child_to_remove;
            });
            break;
        case Object::Type::kScreen:
            std::erase_if(children_screen_, [child_to_remove](const std::unique_ptr<ObjectScreen>& child) {
                return child.get() == child_to_remove;
            });
            break;
        default:
            std::erase_if(children_, [child_to_remove](const std::unique_ptr<Object>& child) {
                return child.get() == child_to_remove;
            });
            break;
    }
}

}  // namespace sdl3
}  // namespace pyc
