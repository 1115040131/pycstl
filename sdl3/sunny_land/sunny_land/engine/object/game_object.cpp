#include "sunny_land/engine/object/game_object.h"

namespace pyc::sunny_land {

GameObject::GameObject(std::string_view name, std::string_view tag) : name_(name), tag_(tag) {
    spdlog::trace("GameObject created: {} {}", name_, tag_);
}

void GameObject::handleInput(Context& context) {
    for (auto& [_, component] : components_) {
        component->handleInput(context);
    }
}

void GameObject::update(std::chrono::duration<double> delta_time, Context& context) {
    for (auto& [_, component] : components_) {
        component->update(delta_time, context);
    }
}

void GameObject::render(Context& context) {
    for (auto& [_, component] : components_) {
        component->render(context);
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