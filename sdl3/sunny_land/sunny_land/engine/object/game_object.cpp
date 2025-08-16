#include "sunny_land/engine/object/game_object.h"

namespace pyc::sunny_land {

GameObject::GameObject(std::string_view name, std::string_view tag) : name_(name), tag_(tag) {
    spdlog::trace("GameObject created: {} {}", name_, tag_);
}

void GameObject::handleInput() {
    for (auto& [_, component] : components_) {
        component->handleInput();
    }
}

void GameObject::update() {
    for (auto& [_, component] : components_) {
        component->update();
    }
}

void GameObject::render() {
    for (auto& [_, component] : components_) {
        component->render();
    }
}

void GameObject::clean() {
    spdlog::trace("Cleaning GameObject \"{}\" \"{}\" ...", name_, tag_);
    for (auto& [_, component] : components_) {
        component->clean();
    }
    components_.clear();
}

}  // namespace pyc::sunny_land