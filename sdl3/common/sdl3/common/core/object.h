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

    virtual void addChild(const std::shared_ptr<Object>& child) { children_.push_back(child); }
    virtual void removeChild(const std::shared_ptr<Object>& child) { std::erase(children_, child); }

protected:
    std::string name_;
    Type type_ = Type::kCommon;
    Game& game_ = Game::GetInstance();
    bool is_active_{true};
    std::vector<std::shared_ptr<Object>> children_;
};

}  // namespace sdl3
}  // namespace pyc
