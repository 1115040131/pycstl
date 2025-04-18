#pragma once

#include <vector>

#include <glm/glm.hpp>

#include "sdl3/common/core/object.h"
#include "sdl3/common/core/object_screen.h"
#include "sdl3/common/core/object_world.h"

namespace pyc {
namespace sdl3 {

class Scene : public Object {
public:
    virtual void clean() override;

    virtual void handleEvents(const SDL_Event& event) override;
    virtual void update(std::chrono::duration<float> delta) override;
    virtual void render() override;

    const glm::vec2& getWorldSize() const { return world_size_; }
    void setWorldSize(const glm::vec2& world_size) { world_size_ = world_size; }

    const glm::vec2& getCameraPosition() const { return camera_position_; }
    void setCameraPosition(const glm::vec2& camera_position);

    const std::vector<std::unique_ptr<ObjectWorld>>& getChildrenWorld() const { return children_world_; }
    const std::vector<std::unique_ptr<ObjectScreen>>& getChildrenScreen() const { return children_screen_; }

    void pause() { is_pause_ = true; }
    void resume() { is_pause_ = false; }

    glm::vec2 worldToScreen(const glm::vec2& position) const { return position - camera_position_; }
    glm::vec2 screenToWorld(const glm::vec2& screen_position) const { return screen_position + camera_position_; }

    virtual Object* addChild(std::unique_ptr<Object> child) override;
    virtual void removeChild(Object* child_to_remove) override;

#ifdef DEBUG_MODE
    virtual void printChildren(int indent = 0) override {
        fmt::println("{:{}}{}:", "", indent, name_);
        for (const auto& child : children_) {
            child->printChildren(indent + 4);
        }
        for (const auto& child : children_world_) {
            child->printChildren(indent + 4);
        }
        for (const auto& child : children_screen_) {
            child->printChildren(indent + 4);
        }
    }
#endif

protected:
    glm::vec2 world_size_;
    glm::vec2 camera_position_;
    std::vector<std::unique_ptr<ObjectWorld>> children_world_;
    std::vector<std::unique_ptr<ObjectScreen>> children_screen_;
    bool is_pause_{};
};

}  // namespace sdl3
}  // namespace pyc
