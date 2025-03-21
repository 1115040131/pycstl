#include "sdl3/common/core/object.h"

namespace pyc {
namespace sdl3 {

void Object::clean() {
    for (auto& child : children_) {
        child->clean();
    }
    children_.clear();
}

void Object::handleEvents(SDL_Event& event) {
    for (auto& child : children_) {
        child->handleEvents(event);
    }
}

void Object::update(std::chrono::duration<float> delta) {
    for (auto& child : children_) {
        child->update(delta);
    }
}

void Object::render() {
    for (auto& child : children_) {
        child->render();
    }
}

}  // namespace sdl3
}  // namespace pyc
