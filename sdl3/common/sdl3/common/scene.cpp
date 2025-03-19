#include "sdl3/common/scene.h"

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
        child->handleEvents(event);
    }
    for (auto& child : children_world_) {
        child->handleEvents(event);
    }
}

void Scene::update(std::chrono::duration<float> delta) {
    Object::update(delta);
    for (auto& child : children_world_) {
        child->update(delta);
    }
    for (auto& child : children_screen_) {
        child->update(delta);
    }
}

void Scene::render() {
    Object::render();
    for (auto& child : children_world_) {
        child->render();
    }
    for (auto& child : children_screen_) {
        child->render();
    }
}

void Scene::setCameraPosition(const glm::vec2& camera_position) {
    constexpr auto kOffset = glm::vec2(30.F);
    camera_position_ = glm::clamp(camera_position, -kOffset, world_size_ - game_.getScreenSize() + kOffset);
}

void Scene::addChild(const std::shared_ptr<Object>& child) {
    switch (child->getType()) {
        case Object::Type::kWorld:
            children_world_.push_back(std::static_pointer_cast<ObjectWorld>(child));
            break;
        case Object::Type::kScreen:
            children_screen_.push_back(std::static_pointer_cast<ObjectScreen>(child));
            break;
        default:
            children_.push_back(child);
            break;
    }
}

void Scene::removeChild(const std::shared_ptr<Object>& child) {
    switch (child->getType()) {
        case Object::Type::kWorld:
            std::erase(children_world_, child);
            break;
        case Object::Type::kScreen:
            std::erase(children_screen_, child);
            break;
        default:
            std::erase(children_, child);
            break;
    }
}

}  // namespace sdl3
}  // namespace pyc
