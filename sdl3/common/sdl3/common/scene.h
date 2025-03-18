#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "sdl3/common/object.h"

namespace pyc {
namespace sdl3 {

class Scene : public Object {
public:
    virtual void init() override {}
    virtual void clean() override {}

    virtual void handleEvents(SDL_Event& event) override {}
    virtual void update(std::chrono::duration<float> delta) override {}
    virtual void render() override {}

protected:
    glm::vec2 camera_position_;
    std::vector<Object*> objects_;
};

}  // namespace sdl3
}  // namespace pyc
