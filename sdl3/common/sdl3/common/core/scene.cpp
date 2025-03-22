#include "sdl3/common/core/scene.h"

namespace pyc {
namespace sdl3 {

void Scene::clean() {
    Clean(children_);
    Clean(children_world_);
    Clean(children_screen_);
}

void Scene::handleEvents(const SDL_Event& event) {
    HandleEvents(children_, event);
    HandleEvents(children_screen_, event);
    HandleEvents(children_world_, event);
}

void Scene::update(std::chrono::duration<float> delta) {
    Update(children_, delta);
    Update(children_world_, delta);
    Update(children_screen_, delta);
}

void Scene::render() {
    Render(children_);
    Render(children_world_);
    Render(children_screen_);
}

void Scene::setCameraPosition(const glm::vec2& camera_position) {
    constexpr auto kOffset = glm::vec2(30.F);
    camera_position_ = glm::clamp(camera_position, -kOffset, world_size_ - game_.getScreenSize() + kOffset);
}

Object* Scene::addChild(std::unique_ptr<Object> child) {
    child->setParent(this);
    switch (child->getType()) {
        case Object::Type::kWorld:
            children_world_.emplace_back(static_cast<ObjectWorld*>(child.release()));
            return children_world_.back().get();
        case Object::Type::kScreen:
            children_screen_.emplace_back(static_cast<ObjectScreen*>(child.release()));
            return children_screen_.back().get();
        default:
            children_.push_back(std::move(child));
            break;
    }
    return children_.back().get();
}

void Scene::removeChild(Object* child_to_remove) {
    switch (child_to_remove->getType()) {
        case Object::Type::kWorld: {
            auto iter = std::ranges::find_if(children_world_,
                                             [child_to_remove](const std::unique_ptr<ObjectWorld>& child) {
                                                 return child.get() == child_to_remove;
                                             });
            if (iter != children_world_.end()) {
                (*iter)->setNeedRemove(true);
            }
        } break;
        case Object::Type::kScreen: {
            auto iter = std::ranges::find_if(children_screen_,
                                             [child_to_remove](const std::unique_ptr<ObjectScreen>& child) {
                                                 return child.get() == child_to_remove;
                                             });
            if (iter != children_screen_.end()) {
                (*iter)->setNeedRemove(true);
            }
        } break;
        default:
            Object::removeChild(child_to_remove);
            break;
    }
}

}  // namespace sdl3
}  // namespace pyc
