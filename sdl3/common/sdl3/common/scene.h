#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "sdl3/common/object.h"
#include "sdl3/common/object_screen.h"
#include "sdl3/common/object_world.h"

namespace pyc {
namespace sdl3 {

class Scene : public Object {
public:
    virtual void clean() override;

    virtual void handleEvents(SDL_Event& event) override;
    virtual void update(std::chrono::duration<float> delta) override;
    virtual void render() override;

    const glm::vec2& getWorldSize() const { return world_size_; }
    void setWorldSize(const glm::vec2& world_size) { world_size_ = world_size; }

    const glm::vec2& getCameraPosition() const { return camera_position_; }
    void setCameraPosition(const glm::vec2& camera_position);

    glm::vec2 worldToScreen(const glm::vec2& position) const { return position - camera_position_; }
    glm::vec2 screenToWorld(const glm::vec2& screen_position) const { return screen_position + camera_position_; }

    virtual void addChild(const std::shared_ptr<Object>& child) override;
    virtual void removeChild(const std::shared_ptr<Object>& child) override;

protected:
    glm::vec2 world_size_;
    glm::vec2 camera_position_;
    std::vector<std::shared_ptr<ObjectWorld>> children_world_;
    std::vector<std::shared_ptr<ObjectScreen>> children_screen_;
};

}  // namespace sdl3
}  // namespace pyc
