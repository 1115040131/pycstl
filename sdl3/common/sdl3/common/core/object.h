#pragma once

#include <chrono>
#include <memory>
#include <vector>

#include <SDL3/SDL.h>

#include "sdl3/common/core/game.h"

namespace pyc {
namespace sdl3 {

class Object {
public:
    enum class Type {
        kCommon,
        kScreen,
        kWorld,
    };

    Object() = default;
    virtual ~Object() = default;

    virtual void init() {}
    virtual void clean();

    virtual void handleEvents(SDL_Event& event);
    virtual void update(std::chrono::duration<float> delta);
    virtual void render();

    void setName(const std::string& name) { name_ = name; }
    const std::string& getName() const { return name_; }

    Type getType() const { return type_; }

    bool isActive() const { return is_active_; }
    void setActive(bool active) { is_active_ = active; }

    bool needRemove() const { return need_remove_; }
    void setNeedRemove(bool need_remove) { need_remove_ = need_remove; }

    Object* getParent() const { return parent_; }
    void setParent(Object* parent) { parent_ = parent; }

    virtual Object* addChild(std::unique_ptr<Object> child);
    virtual void removeChild(Object* child_to_remove);

protected:
    std::string name_;
    Type type_ = Type::kCommon;
    Game& game_ = Game::GetInstance();
    bool is_active_{true};
    bool need_remove_{};
    Object* parent_{};
    std::vector<std::unique_ptr<Object>> children_;
};

template <typename T>
concept DerivedFromObject = std::derived_from<T, Object>;

template <DerivedFromObject T>
void Clean(std::vector<std::unique_ptr<T>>& children) {
    for (auto& child : children) {
        child->clean();
    }
    children.clear();
}

template <DerivedFromObject T>
void HandleEvents(std::vector<std::unique_ptr<T>>& children, SDL_Event& event) {
    for (auto& child : children) {
        if (child->isActive()) {
            child->handleEvents(event);
        }
    }
}

template <DerivedFromObject T>
void Update(std::vector<std::unique_ptr<T>>& children, std::chrono::duration<float> delta) {
    auto partition_it =
        std::partition(children.begin(), children.end(), [delta](const std::unique_ptr<T>& child) {
            if (child->needRemove()) {
                child->clean();
                return false;
            } else if (child->isActive()) {
                child->update(delta);
            }
            return true;
        });
    children.erase(partition_it, children.end());
}

template <DerivedFromObject T>
void Render(std::vector<std::unique_ptr<T>>& children) {
    for (auto& child : children) {
        if (child->isActive()) {
            child->render();
        }
    }
}

}  // namespace sdl3
}  // namespace pyc
