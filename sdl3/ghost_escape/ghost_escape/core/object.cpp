module;

#include <algorithm>
#include <chrono>
#include <memory>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

#ifdef DEBUG_MODE
#include <cstdlib>

#if defined(__unix__) && __has_include(<cxxabi.h>)
#include <cxxabi.h>
#endif

#include <fmt/format.h>
#endif

module ghost_escape.core;

namespace pyc::sdl3 {

#ifdef DEBUG_MODE

std::string Demangle(const char* name) {
#if defined(__unix__) && __has_include(<cxxabi.h>)
    if (char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, nullptr)) {
        std::string result = demangled;
        std::free(demangled);
        return result;
    }
#endif
    return name;
}

#endif  // DEBUG_MODE

void Object::clean() { Clean(children_); }

bool Object::handleEvents(const SDL_Event& event) { return HandleEvents(children_, event); }

void Object::update(std::chrono::duration<float> delta) {
    for (auto& child : object_to_add_) {
        addChild(std::move(child));
    }
    object_to_add_.clear();
    Update(children_, delta);
}

void Object::render() { Render(children_); }

Object* Object::safeAddChild(std::unique_ptr<Object> child) {
    object_to_add_.push_back(std::move(child));
    return object_to_add_.back().get();
}

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
    } else {
        iter = std::ranges::find_if(object_to_add_, [child_to_remove](const std::unique_ptr<Object>& child) {
            return child.get() == child_to_remove;
        });
        if (iter != object_to_add_.end()) {
            object_to_add_.erase(iter);
        }
    }
}

}  // namespace pyc::sdl3
