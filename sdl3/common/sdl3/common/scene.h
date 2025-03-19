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

    const glm::vec2& getWorldSize() const { return world_size_; }
    void setWorldSize(const glm::vec2& world_size) { world_size_ = world_size; }

    const glm::vec2& getCameraPosition() const { return camera_position_; }
    void setCameraPosition(const glm::vec2& camera_position);

    glm::vec2 worldToScreen(const glm::vec2& position) const { return position - camera_position_; }
    glm::vec2 screenToWorld(const glm::vec2& screen_position) const { return screen_position + camera_position_; }

protected:
    glm::vec2 world_size_;
    glm::vec2 camera_position_;
    std::vector<Object*> objects_;
};

}  // namespace sdl3
}  // namespace pyc
