#include "sdl3/common/core/object.h"

namespace pyc {
namespace sdl3 {

void Object::clean() { Clean(children_); }

void Object::handleEvents(SDL_Event& event) { HandleEvents(children_, event); }

void Object::update(std::chrono::duration<float> delta) { Update(children_, delta); }

void Object::render() { Render(children_); }

Object* Object::addChild(std::unique_ptr<Object> child) {
    child->setParent(this);
    children_.push_back(std::move(child));
    return children_.back().get();
}

void Object::removeChild(Object* child_to_remove) {
    auto iter = std::ranges::find_if(children_, [child_to_remove](const std::unique_ptr<Object>& child) {
        return child.get() == child_to_remove;
    });
    if (iter != children_.end()) {
        (*iter)->need_remove_ = true;
    }
}

}  // namespace sdl3
}  // namespace pyc
